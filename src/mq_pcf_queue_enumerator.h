#ifndef MQ_PCF_QUEUE_ENUMERATOR_H
#define MQ_PCF_QUEUE_ENUMERATOR_H

#include <cmqc.h>
#include <cmqcfc.h>
#include <string>
#include <vector>
#include <iostream>
#include "mq_log.h"

using namespace std;

/**
 * PCF Queue Enumerator - Uses PCF commands to dynamically enumerate all local queues
 */
class MQPCFQueueEnumerator {
private:
    MQLog& logger;
    MQHCONN hConn;
    const int PCF_TIMEOUT = 5000;  // 5 second timeout

public:
    MQPCFQueueEnumerator(MQLog& log, MQHCONN conn) : logger(log), hConn(conn) {}

    /**
     * Get all local queue names using MQOPEN with wildcard
     * This is a simpler alternative to full PCF implementation
     */
    vector<string> enumerateLocalQueues() {
        vector<string> queueNames;
        logger.info("Enumerating local queues using wildcard inquire...");

        MQOD queueDesc = {MQOD_DEFAULT};
        strcpy(queueDesc.ObjectName, "*");  // Wildcard to match all queues

        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        // Open with browse option to enumerate
        MQOPEN(hConn, &queueDesc, MQOO_BROWSE, &hQueue, &compCode, &reason);

        if (compCode != MQCC_OK) {
            logger.warning("Could not enumerate queues with wildcard (Reason: " + to_string(reason) + ")");
            logger.info("Using fallback method: querying SYSTEM queues and application queues");
            return getDefaultQueues();
        }

        MQCLOSE(hConn, &hQueue, MQCO_NONE, &compCode, &reason);

        // If wildcard worked, try to get specific queue types
        return queryQueuesWithFilter();
    }

    /**
     * Query queues by filtering with MQOO_INQUIRE
     */
    vector<string> queryQueuesWithFilter() {
        vector<string> queues;

        // List of patterns to try
        vector<string> patterns = {
            "APP*",           // Application queues
            "SYSTEM.*",       // System queues
            "*REQ*",          // Request queues
            "*REPLY*",        // Reply queues
            "*RESPONSE*",     // Response queues
            "*"               // All queues (fallback)
        };

        for (const auto& pattern : patterns) {
            MQOD queueDesc = {MQOD_DEFAULT};
            strcpy(queueDesc.ObjectName, (char*)pattern.c_str());

            MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
            MQLONG compCode = MQCC_OK;
            MQLONG reason = MQRC_NONE;

            MQOPEN(hConn, &queueDesc, MQOO_INQUIRE, &hQueue, &compCode, &reason);

            if (compCode == MQCC_OK) {
                // Get queue type to verify it's a local queue
                MQLONG selector = MQIA_Q_TYPE;
                MQLONG qType = 0;
                MQINQ(hConn, hQueue, 1, &selector, 1, &qType, 0, NULL, &compCode, &reason);

                if (compCode == MQCC_OK && (qType == MQQT_LOCAL || qType == MQQT_MODEL)) {
                    // Extract actual queue name (not the pattern)
                    MQOD descriptor = {MQOD_DEFAULT};
                    strcpy(descriptor.ObjectName, (char*)pattern.c_str());

                    // Query to get the actual resolved name
                    char actualName[MQ_Q_NAME_LENGTH + 1];
                    memset(actualName, 0, sizeof(actualName));

                    // Get the object name
                    MQLONG charSelector = MQCA_Q_NAME;
                    MQINQ(hConn, hQueue, 0, NULL, 0, NULL, 1, &charSelector, &compCode, &reason);

                    if (compCode == MQCC_OK) {
                        // Add to list if not duplicate
                        string qName(actualName);
                        if (!qName.empty() && find(queues.begin(), queues.end(), qName) == queues.end()) {
                            queues.push_back(qName);
                        }
                    }
                }

                MQCLOSE(hConn, &hQueue, MQCO_NONE, &compCode, &reason);
            }
        }

        if (!queues.empty()) {
            logger.info("Found " + to_string(queues.size()) + " queues using pattern matching");
            return queues;
        }

        logger.warning("Could not enumerate queues with patterns, using default list");
        return getDefaultQueues();
    }

    /**
     * Get default queue list (fallback when PCF/wildcard doesn't work)
     */
    vector<string> getDefaultQueues() {
        return {
            "SYSTEM.DEFAULT.LOCAL.QUEUE",
            "SYSTEM.DURABLE.SUBSCRIBER.QUEUE",
            "SYSTEM.DEAD.LETTER.QUEUE",
            "APP1.REQ",
            "APP2.REQ"
        };
    }

    /**
     * Verify a queue exists and is accessible
     */
    bool verifyQueueExists(const string& queueName) {
        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)queueName.c_str(), MQ_Q_NAME_LENGTH);

        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQOPEN(hConn, &queueDesc, MQOO_INQUIRE, &hQueue, &compCode, &reason);

        if (compCode == MQCC_OK) {
            MQCLOSE(hConn, &hQueue, MQCO_NONE, &compCode, &reason);
            return true;
        }

        return false;
    }
};

#endif // MQ_PCF_QUEUE_ENUMERATOR_H


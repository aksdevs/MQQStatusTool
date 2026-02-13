#ifndef MQ_QUEUE_STATUS_H
#define MQ_QUEUE_STATUS_H

#include <cmqc.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstring>
#include "mq_log.h"

struct QueueStatus {
    std::string queueName;
    MQLONG currentDepth;
    MQLONG openInputCount;
    MQLONG openOutputCount;
    std::string queueType;
    std::string status;
    std::string connectionName;
    std::string userId;
    std::string channelName;
    MQLONG processId;
    std::string handleType;
};

class MQQueueStatus {
private:
    MQLog& logger;
    MQHCONN hConn;

public:
    MQQueueStatus(MQLog& log, MQHCONN conn) : logger(log), hConn(conn) {}

    bool getQueueStatus(const std::string& queueName, QueueStatus& status) {
        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, queueName.c_str(), MQ_Q_NAME_LENGTH);

        MQOPEN(hConn, &queueDesc, MQOO_INQUIRE, &hQueue, &compCode, &reason);

        if (compCode != MQCC_OK) {
            logger.warning("Could not open queue for inquiry: " + queueName +
                           " (Reason: " + std::to_string(reason) + ")");
            status.queueName = queueName;
            status.status = "Unavailable";
            status.currentDepth = 0;
            status.openInputCount = 0;
            status.openOutputCount = 0;
            status.connectionName = "N/A";
            status.userId = "N/A";
            status.channelName = "N/A";
            status.processId = 0;
            status.handleType = "N/A";
            return false;
        }

        status.queueName = queueName;

        // Get current queue depth
        MQLONG selector = MQIA_CURRENT_Q_DEPTH;
        MQLONG depth = 0;
        MQINQ(hConn, hQueue, 1, &selector, 1, &depth, 0, NULL, &compCode, &reason);
        status.currentDepth = (compCode == MQCC_OK) ? depth : 0;

        // Get open input count
        selector = MQIA_OPEN_INPUT_COUNT;
        MQLONG openInput = 0;
        MQINQ(hConn, hQueue, 1, &selector, 1, &openInput, 0, NULL, &compCode, &reason);
        status.openInputCount = (compCode == MQCC_OK) ? openInput : 0;

        // Get open output count
        selector = MQIA_OPEN_OUTPUT_COUNT;
        MQLONG openOutput = 0;
        MQINQ(hConn, hQueue, 1, &selector, 1, &openOutput, 0, NULL, &compCode, &reason);
        status.openOutputCount = (compCode == MQCC_OK) ? openOutput : 0;

        // Determine handle type
        if (status.openInputCount > 0 && status.openOutputCount > 0)
            status.handleType = "READ/WRITE";
        else if (status.openInputCount > 0)
            status.handleType = "READ";
        else if (status.openOutputCount > 0)
            status.handleType = "WRITE";
        else
            status.handleType = "N/A";

        // Get queue type
        selector = MQIA_Q_TYPE;
        MQLONG qType = 0;
        MQINQ(hConn, hQueue, 1, &selector, 1, &qType, 0, NULL, &compCode, &reason);
        if (compCode == MQCC_OK) {
            switch (qType) {
                case MQQT_LOCAL:  status.queueType = "LOCAL"; break;
                case MQQT_MODEL:  status.queueType = "MODEL"; break;
                case MQQT_ALIAS:  status.queueType = "ALIAS"; break;
                case MQQT_REMOTE: status.queueType = "REMOTE"; break;
                default:          status.queueType = "UNKNOWN"; break;
            }
        } else {
            status.queueType = "N/A";
        }

        status.connectionName = "N/A";
        status.userId = "N/A";
        status.channelName = "N/A";
        status.processId = 0;
        status.status = "Active";

        MQCLOSE(hConn, &hQueue, MQCO_NONE, &compCode, &reason);
        return true;
    }
};

#endif // MQ_QUEUE_STATUS_H

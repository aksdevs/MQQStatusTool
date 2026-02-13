#ifndef MQ_PCF_HANDLER_H
#define MQ_PCF_HANDLER_H

#include <cmqc.h>
#include <cmqcfc.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <iomanip>

using namespace std;

/**
 * Queue Status Information Structure
 */
struct QueueStatusInfo {
    char queueName[MQ_Q_NAME_LENGTH + 1];
    char applicationTag[MQ_APPL_TAG_LENGTH + 1];
    char connectionName[MQ_CONN_NAME_LENGTH + 1];
    char channelName[MQ_CHANNEL_NAME_LENGTH + 1];
    char userId[MQ_USER_ID_LENGTH + 1];
    MQLONG currentDepth;
    MQLONG ipProcs;          // Input processes
    MQLONG opProcs;          // Output processes
    MQLONG processId;
    MQLONG openInputCount;
    MQLONG openOutputCount;
    char timestamp[50];
    char status[100];
};

/**
 * PCF Handler - Handles PCF communication with queue manager
 */
namespace PCFHandler {

    /**
     * Get queue depth using MQINQ
     */
    inline MQLONG getQueueDepth(MQHCONN hConn, const char* queueName, MQLONG& depth)
    {
        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)queueName, MQ_Q_NAME_LENGTH);

        // Open queue for inquiry
        MQOPEN(hConn, &queueDesc, MQOO_INQUIRE, &hQueue, &CompCode, &Reason);

        if (CompCode == MQCC_OK) {
            // Inquire current queue depth
            // Parameters: Handle, Object, IntSelCount, IntSelectors, IntAttrCount, IntAttrs, CharSelCount, CharSelectors, CompCode, Reason
            MQLONG selector = MQIA_CURRENT_Q_DEPTH;
            MQLONG intAttrCount = 1;  // We're querying 1 integer attribute
            MQINQ(hConn, hQueue, 1, &selector, intAttrCount, &depth, 0, NULL, &CompCode, &Reason);
            MQCLOSE(hConn, &hQueue, MQCO_NONE, &CompCode, &Reason);
        } else {
            depth = 0;
        }

        return Reason;
    }

    /**
     * Get queue status information
     */
    inline MQLONG getQueueStatus(MQHCONN hConn, const char* queueName, QueueStatusInfo& statusInfo)
    {
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        // Initialize status info
        memset(&statusInfo, 0, sizeof(QueueStatusInfo));
        strncpy(statusInfo.queueName, queueName, MQ_Q_NAME_LENGTH);
        strncpy(statusInfo.status, "Active", sizeof(statusInfo.status) - 1);

        // Get queue depth
        MQLONG depth = 0;
        getQueueDepth(hConn, queueName, depth);
        statusInfo.currentDepth = depth;

        // Set defaults/N/A for fields that may not be available via simple MQINQ
        strncpy(statusInfo.applicationTag, "N/A", sizeof(statusInfo.applicationTag) - 1);
        strncpy(statusInfo.connectionName, "N/A", sizeof(statusInfo.connectionName) - 1);
        strncpy(statusInfo.channelName, "N/A", sizeof(statusInfo.channelName) - 1);
        strncpy(statusInfo.userId, "N/A", sizeof(statusInfo.userId) - 1);

        statusInfo.ipProcs = 0;
        statusInfo.opProcs = 0;
        statusInfo.processId = 0;
        statusInfo.openInputCount = 0;
        statusInfo.openOutputCount = 0;

        // For full PCF query, we would need to send command to SYSTEM.ADMIN.COMMAND.Q
        // This is a placeholder that uses available MQINQ data

        return MQRC_NONE;
    }

    /**
     * Query all local queues and their status
     */
    inline MQLONG getAllQueueStatus(MQHCONN hConn, vector<QueueStatusInfo>& queueStatuses, ofstream& logFile)
    {
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        // Get a specific queue's status (the one we have open)
        // In a real scenario, you would iterate through all queues

        return MQRC_NONE;
    }

} // namespace PCFHandler

#endif // MQ_PCF_HANDLER_H


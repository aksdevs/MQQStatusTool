#ifndef MQ_OPERATIONS_H
#define MQ_OPERATIONS_H

#include <cmqc.h>
#include <cmqcfc.h>
#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

/**
 * MQ Operations - High-level functions for common MQ operations
 */

namespace MQOps {

    /**
     * Put a message on a queue
     */
    inline MQLONG putMessage(MQHCONN hConn, const char* queueName,
                             const char* messageData, MQLONG messageLength)
    {
        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)queueName, MQ_Q_NAME_LENGTH);

        // Open queue for output
        MQOPEN(hConn, &queueDesc, MQOO_OUTPUT, &hQueue, &CompCode, &Reason);
        if (CompCode != MQCC_OK) {
            return Reason;
        }

        // Create and send message
        MQPMO putMsgOpts = {MQPMO_DEFAULT};
        MQMD msgDesc = {MQMD_DEFAULT};

        MQPUT(hConn, hQueue, &msgDesc, &putMsgOpts, messageLength,
              (void*)messageData, &CompCode, &Reason);

        MQCLOSE(hConn, &hQueue, MQCO_NONE, &CompCode, &Reason);

        return Reason;
    }

    /**
     * Get a message from a queue with timeout
     */
    inline MQLONG getMessage(MQHCONN hConn, const char* queueName,
                             unsigned char* buffer, MQLONG bufferSize,
                             MQLONG& dataLength, MQLONG timeout)
    {
        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)queueName, MQ_Q_NAME_LENGTH);

        // Open queue for input
        MQOPEN(hConn, &queueDesc, MQOO_INPUT_AS_Q_DEF, &hQueue, &CompCode, &Reason);
        if (CompCode != MQCC_OK) {
            return Reason;
        }

        MQGMO getMsgOpts = {MQGMO_DEFAULT};
        getMsgOpts.Options = MQGMO_WAIT;
        getMsgOpts.WaitInterval = timeout;

        MQMD msgDesc = {MQMD_DEFAULT};

        MQGET(hConn, hQueue, &msgDesc, &getMsgOpts, bufferSize,
              buffer, &dataLength, &CompCode, &Reason);

        MQCLOSE(hConn, &hQueue, MQCO_NONE, &CompCode, &Reason);

        return Reason;
    }

} // namespace MQOps

#endif // MQ_OPERATIONS_H


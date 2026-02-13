#ifndef MQ_DYNAMIC_QUEUE_H
#define MQ_DYNAMIC_QUEUE_H

#include <cmqc.h>
#include <cstring>
#include <string>
#include <iostream>

using namespace std;

/**
 * Dynamic Queue Handler - Create and manage temporary reply queues for PCF
 */
namespace DynamicQueue {

    /**
     * Create a dynamic reply queue
     * Returns the queue name if successful, empty string if failed
     */
    inline string createDynamicReplyQueue(MQHCONN hConn, const string& baseQueueName = "REPLY.*") {
        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        // Create object descriptor for dynamic queue
        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)baseQueueName.c_str(), MQ_Q_NAME_LENGTH);
        strncpy(queueDesc.DynamicQName, "DYN.REPLY.*", (size_t)MQ_Q_NAME_LENGTH);

        // Open (create) the dynamic queue from model queue
        MQOPEN(hConn, &queueDesc, MQOO_INPUT_EXCLUSIVE, &hQueue, &CompCode, &Reason);

        if (CompCode != MQCC_OK) {
            return "";  // Failed to create
        }

        // Close the queue (it's created but we'll open it for reading responses later)
        MQCLOSE(hConn, &hQueue, MQCO_NONE, &CompCode, &Reason);

        // Return the dynamically generated queue name
        return string(queueDesc.DynamicQName);
    }

    /**
     * Delete a dynamic reply queue
     */
    inline MQLONG deleteDynamicReplyQueue(MQHCONN hConn, const string& queueName) {
        if (queueName.empty()) {
            return MQRC_NONE;  // Nothing to delete
        }

        MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG CompCode = MQCC_OK;
        MQLONG Reason = MQRC_NONE;

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, (char*)queueName.c_str(), MQ_Q_NAME_LENGTH);

        // Open the queue for reading/writing
        MQOPEN(hConn, &queueDesc, MQOO_INPUT_AS_Q_DEF, &hQueue, &CompCode, &Reason);

        if (CompCode == MQCC_OK) {
            // Queue will be deleted when closed with MQCO_DELETE_PURGE
            MQCLOSE(hConn, &hQueue, MQCO_DELETE_PURGE, &CompCode, &Reason);
        }

        return Reason;
    }

}  // namespace DynamicQueue

#endif // MQ_DYNAMIC_QUEUE_H


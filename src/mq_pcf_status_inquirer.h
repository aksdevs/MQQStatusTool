#ifndef MQ_PCF_STATUS_INQUIRER_H
#define MQ_PCF_STATUS_INQUIRER_H

#include <cmqc.h>
#include <cmqcfc.h>
#include <string>
#include <vector>
#include <cstring>
#include "mq_log.h"

struct PCFQueueData {
    std::string queueName;
    MQLONG currentDepth;
    MQLONG openInputCount;
    MQLONG openOutputCount;
    std::string queueType;
};

class MQPCFStatusInquirer {
private:
    MQLog& logger;
    MQHCONN hConn;

public:
    MQPCFStatusInquirer(MQLog& log, MQHCONN conn) : logger(log), hConn(conn) {}

    std::vector<PCFQueueData> inquireAllQueueStatuses() {
        std::vector<PCFQueueData> queueStatuses;

        logger.info("Sending PCF INQUIRE_Q command to get all local queue attributes...");

        // Open SYSTEM.ADMIN.COMMAND.QUEUE for sending PCF commands
        MQOD cmdQueueDesc = {MQOD_DEFAULT};
        strncpy(cmdQueueDesc.ObjectName, "SYSTEM.ADMIN.COMMAND.QUEUE", MQ_Q_NAME_LENGTH);

        MQHOBJ hCmdQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQOPEN(hConn, &cmdQueueDesc, MQOO_OUTPUT, &hCmdQueue, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to open SYSTEM.ADMIN.COMMAND.QUEUE (Reason: " + std::to_string(reason) + ")");
            return queueStatuses;
        }

        // Create a dynamic reply queue from the model queue
        MQOD replyQueueDesc = {MQOD_DEFAULT};
        strncpy(replyQueueDesc.ObjectName, "SYSTEM.DEFAULT.MODEL.QUEUE", MQ_Q_NAME_LENGTH);
        strncpy(replyQueueDesc.DynamicQName, "PCF.REPLY.*", MQ_Q_NAME_LENGTH);

        MQHOBJ hReplyQueue = MQHO_UNUSABLE_HOBJ;
        MQOPEN(hConn, &replyQueueDesc, MQOO_INPUT_EXCLUSIVE, &hReplyQueue, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to create dynamic reply queue (Reason: " + std::to_string(reason) + ")");
            MQCLOSE(hConn, &hCmdQueue, MQCO_NONE, &compCode, &reason);
            return queueStatuses;
        }

        // The resolved dynamic queue name is in ObjectName after MQOPEN
        char replyQName[MQ_Q_NAME_LENGTH + 1];
        memset(replyQName, 0, sizeof(replyQName));
        memcpy(replyQName, replyQueueDesc.ObjectName, MQ_Q_NAME_LENGTH);

        logger.info("Created dynamic reply queue: " + std::string(replyQName));

        // Build PCF INQUIRE_Q command message
        // Parameters: Q_NAME=* and Q_TYPE=LOCAL
        unsigned char cmdBuffer[4096];
        memset(cmdBuffer, 0, sizeof(cmdBuffer));

        // PCF Header (MQCFH)
        MQCFH* pCFH = (MQCFH*)cmdBuffer;
        pCFH->Type = MQCFT_COMMAND;
        pCFH->StrucLength = MQCFH_STRUC_LENGTH;
        pCFH->Version = MQCFH_VERSION_1;
        pCFH->Command = MQCMD_INQUIRE_Q;
        pCFH->MsgSeqNumber = 1;
        pCFH->Control = MQCFC_LAST;
        pCFH->CompCode = MQCC_OK;
        pCFH->Reason = MQRC_NONE;
        pCFH->ParameterCount = 2;

        int offset = pCFH->StrucLength;

        // Parameter 1: Queue Name filter = "*" (all queues)
        MQCFST* pQName = (MQCFST*)(cmdBuffer + offset);
        pQName->Type = MQCFT_STRING;
        pQName->Parameter = MQCA_Q_NAME;
        pQName->CodedCharSetId = MQCCSI_DEFAULT;
        pQName->StringLength = 1;
        // Align StrucLength to 4-byte boundary
        pQName->StrucLength = MQCFST_STRUC_LENGTH_FIXED + ((1 + 3) & ~3);
        memcpy(pQName->String, "*", 1);
        offset += pQName->StrucLength;

        // Parameter 2: Queue Type = LOCAL
        MQCFIN* pQType = (MQCFIN*)(cmdBuffer + offset);
        pQType->Type = MQCFT_INTEGER;
        pQType->StrucLength = MQCFIN_STRUC_LENGTH;
        pQType->Parameter = MQIA_Q_TYPE;
        pQType->Value = MQQT_LOCAL;
        offset += pQType->StrucLength;

        // Build message descriptor for PCF command
        MQMD cmdMsgDesc = {MQMD_DEFAULT};
        memcpy(cmdMsgDesc.Format, MQFMT_ADMIN, sizeof(cmdMsgDesc.Format));
        cmdMsgDesc.MsgType = MQMT_REQUEST;
        strncpy(cmdMsgDesc.ReplyToQ, replyQName, MQ_Q_NAME_LENGTH);

        MQPMO putMsgOpts = {MQPMO_DEFAULT};

        MQPUT(hConn, hCmdQueue, &cmdMsgDesc, &putMsgOpts, offset, cmdBuffer, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to send PCF command (Reason: " + std::to_string(reason) + ")");
            MQCLOSE(hConn, &hCmdQueue, MQCO_NONE, &compCode, &reason);
            MQCLOSE(hConn, &hReplyQueue, MQCO_DELETE_PURGE, &compCode, &reason);
            return queueStatuses;
        }

        logger.info("PCF command sent successfully, waiting for responses...");

        // Read PCF response messages
        bool lastMessage = false;
        while (!lastMessage) {
            unsigned char replyBuffer[65536];
            memset(replyBuffer, 0, sizeof(replyBuffer));

            MQMD replyMsgDesc = {MQMD_DEFAULT};
            MQGMO getMsgOpts = {MQGMO_DEFAULT};
            getMsgOpts.Options = MQGMO_WAIT | MQGMO_CONVERT;
            getMsgOpts.WaitInterval = 10000; // 10 second timeout

            MQLONG dataLen = 0;
            MQGET(hConn, hReplyQueue, &replyMsgDesc, &getMsgOpts, sizeof(replyBuffer),
                  replyBuffer, &dataLen, &compCode, &reason);

            if (compCode != MQCC_OK) {
                if (reason == MQRC_NO_MSG_AVAILABLE) {
                    logger.info("No more PCF responses (timeout)");
                } else {
                    logger.error("Error reading PCF response (Reason: " + std::to_string(reason) + ")");
                }
                break;
            }

            // Parse PCF response header
            MQCFH* pRespCFH = (MQCFH*)replyBuffer;

            if (pRespCFH->Type != MQCFT_RESPONSE) {
                logger.warning("Unexpected PCF message type: " + std::to_string(pRespCFH->Type));
                continue;
            }

            // Check if this is the last message in the set
            if (pRespCFH->Control == MQCFC_LAST) {
                lastMessage = true;
            }

            // If the response indicates an error, log it but continue
            if (pRespCFH->CompCode == MQCC_FAILED) {
                logger.warning("PCF response error, reason: " + std::to_string(pRespCFH->Reason));
                break;
            }

            // Parse parameters from response
            PCFQueueData currentQueue;
            currentQueue.currentDepth = 0;
            currentQueue.openInputCount = 0;
            currentQueue.openOutputCount = 0;
            currentQueue.queueType = "LOCAL";

            int respOffset = pRespCFH->StrucLength;
            for (int p = 0; p < pRespCFH->ParameterCount && respOffset < dataLen; p++) {
                MQLONG* pType = (MQLONG*)(replyBuffer + respOffset);

                if (*pType == MQCFT_STRING) {
                    MQCFST* pStr = (MQCFST*)(replyBuffer + respOffset);

                    if (pStr->Parameter == MQCA_Q_NAME) {
                        char qName[MQ_Q_NAME_LENGTH + 1];
                        memset(qName, 0, sizeof(qName));
                        int copyLen = pStr->StringLength;
                        if (copyLen > MQ_Q_NAME_LENGTH) copyLen = MQ_Q_NAME_LENGTH;
                        memcpy(qName, pStr->String, copyLen);

                        // Trim trailing spaces
                        std::string trimmedName(qName);
                        size_t endpos = trimmedName.find_last_not_of(" ");
                        if (endpos != std::string::npos)
                            trimmedName = trimmedName.substr(0, endpos + 1);
                        currentQueue.queueName = trimmedName;
                    }

                    respOffset += pStr->StrucLength;
                }
                else if (*pType == MQCFT_INTEGER) {
                    MQCFIN* pInt = (MQCFIN*)(replyBuffer + respOffset);

                    if (pInt->Parameter == MQIA_CURRENT_Q_DEPTH) {
                        currentQueue.currentDepth = pInt->Value;
                    }
                    else if (pInt->Parameter == MQIA_OPEN_INPUT_COUNT) {
                        currentQueue.openInputCount = pInt->Value;
                    }
                    else if (pInt->Parameter == MQIA_OPEN_OUTPUT_COUNT) {
                        currentQueue.openOutputCount = pInt->Value;
                    }
                    else if (pInt->Parameter == MQIA_Q_TYPE) {
                        switch (pInt->Value) {
                            case MQQT_LOCAL:  currentQueue.queueType = "LOCAL"; break;
                            case MQQT_MODEL:  currentQueue.queueType = "MODEL"; break;
                            case MQQT_ALIAS:  currentQueue.queueType = "ALIAS"; break;
                            case MQQT_REMOTE: currentQueue.queueType = "REMOTE"; break;
                            default:          currentQueue.queueType = "UNKNOWN"; break;
                        }
                    }

                    respOffset += pInt->StrucLength;
                }
                else {
                    // Unknown parameter type - advance by StrucLength
                    MQLONG structLen = *(MQLONG*)(replyBuffer + respOffset + sizeof(MQLONG));
                    if (structLen <= 0) break; // safety
                    respOffset += structLen;
                }
            }

            // Add queue data if we got a valid name
            if (!currentQueue.queueName.empty()) {
                queueStatuses.push_back(currentQueue);
            }
        }

        // Close command queue and delete dynamic reply queue
        MQCLOSE(hConn, &hCmdQueue, MQCO_NONE, &compCode, &reason);
        MQCLOSE(hConn, &hReplyQueue, MQCO_DELETE_PURGE, &compCode, &reason);

        logger.info("Retrieved " + std::to_string(queueStatuses.size()) + " queue statuses via PCF");
        return queueStatuses;
    }
};

#endif // MQ_PCF_STATUS_INQUIRER_H

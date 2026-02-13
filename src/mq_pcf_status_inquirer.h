#ifndef MQ_PCF_STATUS_INQUIRER_H
#define MQ_PCF_STATUS_INQUIRER_H

#include <cmqc.h>
#include <cmqcfc.h>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include "mq_log.h"

struct PCFQueueData {
    std::string queueName;
    MQLONG currentDepth;
    MQLONG openInputCount;
    MQLONG openOutputCount;
    std::string queueType;
    std::string connection;
    std::string user;
    std::string applicationTag;
    MQLONG processId;
    std::string channelName;
    std::string processType;  // Application type: "CICS", "BATCH", "USER", etc.
    std::string role;         // "Reader", "Writer", "Reader/Writer", or "N/A"
};

// Internal struct to hold per-handle info from MQCMD_INQUIRE_Q_STATUS with HANDLE status type
struct PCFHandleData {
    std::string queueName;
    std::string connection;
    std::string user;
    std::string applicationTag;
    std::string channelName;
    MQLONG processId;
    MQLONG openOptions;
    std::string processType;  // Application type from MQIACF_APPL_TYPE
    std::string role;         // Derived: "Reader" or "Writer"
};

class MQPCFStatusInquirer {
private:
    MQLog& logger;
    MQHCONN hConn;

    // Helper to trim trailing spaces from MQ fixed-length strings
    static std::string trimMQString(const char* src, int len) {
        std::string s(src, len);
        size_t endpos = s.find_last_not_of(" ");
        if (endpos != std::string::npos)
            return s.substr(0, endpos + 1);
        return "";
    }

    // Send a PCF command and collect all response messages
    std::vector<std::vector<unsigned char>> sendPCFCommand(
        MQHOBJ hCmdQueue, MQHOBJ hReplyQueue, const char* replyQName,
        unsigned char* cmdBuffer, int cmdLen)
    {
        std::vector<std::vector<unsigned char>> responses;

        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQMD cmdMsgDesc = {MQMD_DEFAULT};
        memcpy(cmdMsgDesc.Format, MQFMT_ADMIN, sizeof(cmdMsgDesc.Format));
        cmdMsgDesc.MsgType = MQMT_REQUEST;
        strncpy(cmdMsgDesc.ReplyToQ, replyQName, MQ_Q_NAME_LENGTH);

        MQPMO putMsgOpts = {MQPMO_DEFAULT};

        MQPUT(hConn, hCmdQueue, &cmdMsgDesc, &putMsgOpts, cmdLen, cmdBuffer, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to send PCF command (Reason: " + std::to_string(reason) + ")");
            return responses;
        }

        bool lastMessage = false;
        while (!lastMessage) {
            std::vector<unsigned char> replyBuffer(65536, 0);

            MQMD replyMsgDesc = {MQMD_DEFAULT};
            MQGMO getMsgOpts = {MQGMO_DEFAULT};
            getMsgOpts.Options = MQGMO_WAIT | MQGMO_CONVERT;
            getMsgOpts.WaitInterval = 10000;

            MQLONG dataLen = 0;
            MQGET(hConn, hReplyQueue, &replyMsgDesc, &getMsgOpts,
                  (MQLONG)replyBuffer.size(), replyBuffer.data(), &dataLen, &compCode, &reason);

            if (compCode != MQCC_OK) {
                if (reason == MQRC_NO_MSG_AVAILABLE) {
                    logger.info("No more PCF responses (timeout)");
                } else {
                    logger.error("Error reading PCF response (Reason: " + std::to_string(reason) + ")");
                }
                break;
            }

            MQCFH* pRespCFH = (MQCFH*)replyBuffer.data();
            if (pRespCFH->Type != MQCFT_RESPONSE) {
                logger.warning("Unexpected PCF message type: " + std::to_string(pRespCFH->Type));
                continue;
            }

            if (pRespCFH->Control == MQCFC_LAST) {
                lastMessage = true;
            }

            if (pRespCFH->CompCode == MQCC_FAILED) {
                logger.warning("PCF response error, reason: " + std::to_string(pRespCFH->Reason));
                break;
            }

            replyBuffer.resize(dataLen);
            responses.push_back(std::move(replyBuffer));
        }

        return responses;
    }

    // Build PCF command for INQUIRE_Q_STATUS (queue-level: depth, IPPROCS, OPPROCS)
    int buildQueueStatusCommand(unsigned char* cmdBuffer) {
        memset(cmdBuffer, 0, 4096);

        MQCFH* pCFH = (MQCFH*)cmdBuffer;
        pCFH->Type = MQCFT_COMMAND;
        pCFH->StrucLength = MQCFH_STRUC_LENGTH;
        pCFH->Version = MQCFH_VERSION_1;
        pCFH->Command = MQCMD_INQUIRE_Q_STATUS;
        pCFH->MsgSeqNumber = 1;
        pCFH->Control = MQCFC_LAST;
        pCFH->CompCode = MQCC_OK;
        pCFH->Reason = MQRC_NONE;
        pCFH->ParameterCount = 1;

        int offset = pCFH->StrucLength;

        // Parameter 1: Queue Name = "*"
        MQCFST* pQName = (MQCFST*)(cmdBuffer + offset);
        pQName->Type = MQCFT_STRING;
        pQName->Parameter = MQCA_Q_NAME;
        pQName->CodedCharSetId = MQCCSI_DEFAULT;
        pQName->StringLength = 1;
        pQName->StrucLength = MQCFST_STRUC_LENGTH_FIXED + ((1 + 3) & ~3);
        memcpy(pQName->String, "*", 1);
        offset += pQName->StrucLength;

        // Default StatusType is MQIACF_Q_STATUS which returns queue-level info
        return offset;
    }

    // Build PCF command for INQUIRE_Q_STATUS with StatusType=HANDLE (per-handle details)
    int buildHandleStatusCommand(unsigned char* cmdBuffer) {
        memset(cmdBuffer, 0, 4096);

        MQCFH* pCFH = (MQCFH*)cmdBuffer;
        pCFH->Type = MQCFT_COMMAND;
        pCFH->StrucLength = MQCFH_STRUC_LENGTH;
        pCFH->Version = MQCFH_VERSION_1;
        pCFH->Command = MQCMD_INQUIRE_Q_STATUS;
        pCFH->MsgSeqNumber = 1;
        pCFH->Control = MQCFC_LAST;
        pCFH->CompCode = MQCC_OK;
        pCFH->Reason = MQRC_NONE;
        pCFH->ParameterCount = 2;

        int offset = pCFH->StrucLength;

        // Parameter 1: Queue Name = "*"
        MQCFST* pQName = (MQCFST*)(cmdBuffer + offset);
        pQName->Type = MQCFT_STRING;
        pQName->Parameter = MQCA_Q_NAME;
        pQName->CodedCharSetId = MQCCSI_DEFAULT;
        pQName->StringLength = 1;
        pQName->StrucLength = MQCFST_STRUC_LENGTH_FIXED + ((1 + 3) & ~3);
        memcpy(pQName->String, "*", 1);
        offset += pQName->StrucLength;

        // Parameter 2: StatusType = HANDLE (per-handle info)
        MQCFIN* pStatusType = (MQCFIN*)(cmdBuffer + offset);
        pStatusType->Type = MQCFT_INTEGER;
        pStatusType->StrucLength = MQCFIN_STRUC_LENGTH;
        pStatusType->Parameter = MQIACF_Q_STATUS_TYPE;
        pStatusType->Value = MQIACF_Q_HANDLE;
        offset += pStatusType->StrucLength;

        return offset;
    }

    // Parse a queue-level status response into PCFQueueData
    PCFQueueData parseQueueStatusResponse(const std::vector<unsigned char>& data) {
        PCFQueueData q;
        q.currentDepth = 0;
        q.openInputCount = 0;
        q.openOutputCount = 0;
        q.queueType = "LOCAL";
        q.processId = 0;
        q.connection = "N/A";
        q.user = "N/A";
        q.applicationTag = "N/A";
        q.channelName = "N/A";
        q.processType = "N/A";
        q.role = "N/A";

        MQCFH* pCFH = (MQCFH*)data.data();
        int respOffset = pCFH->StrucLength;
        MQLONG dataLen = (MQLONG)data.size();

        for (int p = 0; p < pCFH->ParameterCount && respOffset < dataLen; p++) {
            MQLONG* pType = (MQLONG*)(data.data() + respOffset);

            if (*pType == MQCFT_STRING) {
                MQCFST* pStr = (MQCFST*)(data.data() + respOffset);
                if (pStr->Parameter == MQCA_Q_NAME) {
                    char buf[MQ_Q_NAME_LENGTH + 1] = {0};
                    int copyLen = pStr->StringLength;
                    if (copyLen > MQ_Q_NAME_LENGTH) copyLen = MQ_Q_NAME_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    q.queueName = trimMQString(buf, copyLen);
                }
                respOffset += pStr->StrucLength;
            }
            else if (*pType == MQCFT_INTEGER) {
                MQCFIN* pInt = (MQCFIN*)(data.data() + respOffset);
                if (pInt->Parameter == MQIA_CURRENT_Q_DEPTH) {
                    q.currentDepth = pInt->Value;
                }
                else if (pInt->Parameter == MQIA_OPEN_INPUT_COUNT) {
                    q.openInputCount = pInt->Value;
                }
                else if (pInt->Parameter == MQIA_OPEN_OUTPUT_COUNT) {
                    q.openOutputCount = pInt->Value;
                }
                else if (pInt->Parameter == MQIA_Q_TYPE) {
                    switch (pInt->Value) {
                        case MQQT_LOCAL:  q.queueType = "LOCAL"; break;
                        case MQQT_MODEL:  q.queueType = "MODEL"; break;
                        case MQQT_ALIAS:  q.queueType = "ALIAS"; break;
                        case MQQT_REMOTE: q.queueType = "REMOTE"; break;
                        default:          q.queueType = "UNKNOWN"; break;
                    }
                }
                respOffset += pInt->StrucLength;
            }
            else {
                MQLONG structLen = *(MQLONG*)(data.data() + respOffset + sizeof(MQLONG));
                if (structLen <= 0) break;
                respOffset += structLen;
            }
        }
        return q;
    }

    // Parse a handle-level status response into PCFHandleData
    PCFHandleData parseHandleStatusResponse(const std::vector<unsigned char>& data) {
        PCFHandleData h;
        h.connection = "N/A";
        h.user = "N/A";
        h.applicationTag = "N/A";
        h.channelName = "N/A";
        h.processId = 0;
        h.openOptions = 0;
        h.processType = "N/A";
        h.role = "N/A";

        MQCFH* pCFH = (MQCFH*)data.data();
        int respOffset = pCFH->StrucLength;
        MQLONG dataLen = (MQLONG)data.size();

        for (int p = 0; p < pCFH->ParameterCount && respOffset < dataLen; p++) {
            MQLONG* pType = (MQLONG*)(data.data() + respOffset);

            if (*pType == MQCFT_STRING) {
                MQCFST* pStr = (MQCFST*)(data.data() + respOffset);
                int copyLen = pStr->StringLength;

                if (pStr->Parameter == MQCA_Q_NAME) {
                    char buf[MQ_Q_NAME_LENGTH + 1] = {0};
                    if (copyLen > MQ_Q_NAME_LENGTH) copyLen = MQ_Q_NAME_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    h.queueName = trimMQString(buf, copyLen);
                }
                else if (pStr->Parameter == MQCACH_CONNECTION_NAME) {
                    char buf[MQ_CONN_NAME_LENGTH + 1] = {0};
                    if (copyLen > MQ_CONN_NAME_LENGTH) copyLen = MQ_CONN_NAME_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    std::string trimmed = trimMQString(buf, copyLen);
                    if (!trimmed.empty()) h.connection = trimmed;
                }
                else if (pStr->Parameter == MQCACF_USER_IDENTIFIER) {
                    char buf[MQ_USER_ID_LENGTH + 1] = {0};
                    if (copyLen > MQ_USER_ID_LENGTH) copyLen = MQ_USER_ID_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    std::string trimmed = trimMQString(buf, copyLen);
                    if (!trimmed.empty()) h.user = trimmed;
                }
                else if (pStr->Parameter == MQCACF_APPL_TAG) {
                    char buf[MQ_APPL_TAG_LENGTH + 1] = {0};
                    if (copyLen > MQ_APPL_TAG_LENGTH) copyLen = MQ_APPL_TAG_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    std::string trimmed = trimMQString(buf, copyLen);
                    if (!trimmed.empty()) h.applicationTag = trimmed;
                }
                else if (pStr->Parameter == MQCACH_CHANNEL_NAME) {
                    char buf[MQ_CHANNEL_NAME_LENGTH + 1] = {0};
                    if (copyLen > MQ_CHANNEL_NAME_LENGTH) copyLen = MQ_CHANNEL_NAME_LENGTH;
                    memcpy(buf, pStr->String, copyLen);
                    std::string trimmed = trimMQString(buf, copyLen);
                    if (!trimmed.empty()) h.channelName = trimmed;
                }

                respOffset += pStr->StrucLength;
            }
            else if (*pType == MQCFT_INTEGER) {
                MQCFIN* pInt = (MQCFIN*)(data.data() + respOffset);

                if (pInt->Parameter == MQIACF_PROCESS_ID) {
                    h.processId = pInt->Value;
                }
                else if (pInt->Parameter == MQIACF_OPEN_OPTIONS) {
                    h.openOptions = pInt->Value;
                }
                else if (pInt->Parameter == MQIA_APPL_TYPE) {
                    switch (pInt->Value) {
                        case MQAT_CICS:             h.processType = "CICS"; break;
                        case MQAT_MVS:              h.processType = "MVS"; break;
                        case MQAT_OS400:            h.processType = "OS400"; break;
                        case MQAT_UNIX:             h.processType = "UNIX"; break;
                        case MQAT_WINDOWS_NT:       h.processType = "WINDOWS"; break;
                        case MQAT_JAVA:             h.processType = "JAVA"; break;
                        case MQAT_CHANNEL_INITIATOR: h.processType = "CHINIT"; break;
                        case MQAT_QMGR:            h.processType = "QMGR"; break;
                        case MQAT_USER:             h.processType = "USER"; break;
                        case MQAT_BROKER:           h.processType = "BROKER"; break;
                        default:                    h.processType = "OTHER(" + std::to_string(pInt->Value) + ")"; break;
                    }
                }

                respOffset += pInt->StrucLength;
            }
            else {
                MQLONG structLen = *(MQLONG*)(data.data() + respOffset + sizeof(MQLONG));
                if (structLen <= 0) break;
                respOffset += structLen;
            }
        }

        // Determine role from open options
        bool isInput = (h.openOptions & MQOO_INPUT_AS_Q_DEF) ||
                       (h.openOptions & MQOO_INPUT_SHARED) ||
                       (h.openOptions & MQOO_INPUT_EXCLUSIVE);
        bool isOutput = (h.openOptions & MQOO_OUTPUT) != 0;

        if (isInput && isOutput)
            h.role = "Reader/Writer";
        else if (isInput)
            h.role = "Reader";
        else if (isOutput)
            h.role = "Writer";

        return h;
    }

public:
    MQPCFStatusInquirer(MQLog& log, MQHCONN conn) : logger(log), hConn(conn) {}

    std::vector<PCFQueueData> inquireAllQueueStatuses() {
        std::vector<PCFQueueData> results;

        logger.info("Sending PCF INQUIRE_Q_STATUS commands for queue-level and handle-level status...");

        // Open command queue
        MQOD cmdQueueDesc = {MQOD_DEFAULT};
        strncpy(cmdQueueDesc.ObjectName, "SYSTEM.ADMIN.COMMAND.QUEUE", MQ_Q_NAME_LENGTH);

        MQHOBJ hCmdQueue = MQHO_UNUSABLE_HOBJ;
        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQOPEN(hConn, &cmdQueueDesc, MQOO_OUTPUT, &hCmdQueue, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to open SYSTEM.ADMIN.COMMAND.QUEUE (Reason: " + std::to_string(reason) + ")");
            return results;
        }

        // Create dynamic reply queue
        MQOD replyQueueDesc = {MQOD_DEFAULT};
        strncpy(replyQueueDesc.ObjectName, "SYSTEM.DEFAULT.MODEL.QUEUE", MQ_Q_NAME_LENGTH);
        strncpy(replyQueueDesc.DynamicQName, "PCF.REPLY.*", MQ_Q_NAME_LENGTH);

        MQHOBJ hReplyQueue = MQHO_UNUSABLE_HOBJ;
        MQOPEN(hConn, &replyQueueDesc, MQOO_INPUT_EXCLUSIVE, &hReplyQueue, &compCode, &reason);
        if (compCode != MQCC_OK) {
            logger.error("Failed to create dynamic reply queue (Reason: " + std::to_string(reason) + ")");
            MQCLOSE(hConn, &hCmdQueue, MQCO_NONE, &compCode, &reason);
            return results;
        }

        char replyQName[MQ_Q_NAME_LENGTH + 1] = {0};
        memcpy(replyQName, replyQueueDesc.ObjectName, MQ_Q_NAME_LENGTH);
        logger.info("Created dynamic reply queue: " + std::string(replyQName));

        // === Step 1: Queue-level status (depth, IPPROCS, OPPROCS) ===
        unsigned char cmdBuffer[4096];
        int cmdLen = buildQueueStatusCommand(cmdBuffer);

        logger.info("Sending queue-level status inquiry...");
        auto queueResponses = sendPCFCommand(hCmdQueue, hReplyQueue, replyQName, cmdBuffer, cmdLen);

        // Parse queue-level data into a map by queue name
        std::map<std::string, PCFQueueData> queueMap;
        for (const auto& resp : queueResponses) {
            PCFQueueData q = parseQueueStatusResponse(resp);
            if (!q.queueName.empty()) {
                queueMap[q.queueName] = q;
            }
        }
        logger.info("Retrieved " + std::to_string(queueMap.size()) + " queue statuses");

        // === Step 2: Handle-level status (per-handle: connection, channel, user, PID, role) ===
        cmdLen = buildHandleStatusCommand(cmdBuffer);

        logger.info("Sending handle-level status inquiry...");
        auto handleResponses = sendPCFCommand(hCmdQueue, hReplyQueue, replyQName, cmdBuffer, cmdLen);

        // Parse handle-level data, grouped by queue name
        std::map<std::string, std::vector<PCFHandleData>> handleMap;
        for (const auto& resp : handleResponses) {
            PCFHandleData h = parseHandleStatusResponse(resp);
            if (!h.queueName.empty()) {
                handleMap[h.queueName].push_back(h);
            }
        }
        logger.info("Retrieved " + std::to_string(handleResponses.size()) + " handle entries");

        // === Step 3: Merge - for each queue, emit one row per handle ===
        for (auto& entry : queueMap) {
            const std::string& qName = entry.first;
            PCFQueueData& baseQueue = entry.second;

            auto it = handleMap.find(qName);
            if (it != handleMap.end() && !it->second.empty()) {
                // Queue has open handles - create one row per handle
                for (const auto& h : it->second) {
                    PCFQueueData row = baseQueue;  // Copy queue-level data
                    row.connection = h.connection;
                    row.user = h.user;
                    row.applicationTag = h.applicationTag;
                    row.channelName = h.channelName;
                    row.processId = h.processId;
                    row.processType = h.processType;
                    row.role = h.role;
                    results.push_back(row);
                }
            } else {
                // No open handles - emit single row with defaults
                results.push_back(baseQueue);
            }
        }

        // Close command queue and delete dynamic reply queue
        MQCLOSE(hConn, &hCmdQueue, MQCO_NONE, &compCode, &reason);
        MQCLOSE(hConn, &hReplyQueue, MQCO_DELETE_PURGE, &compCode, &reason);

        logger.info("Final result: " + std::to_string(results.size()) + " rows (queues + handles)");
        return results;
    }
};

#endif // MQ_PCF_STATUS_INQUIRER_H

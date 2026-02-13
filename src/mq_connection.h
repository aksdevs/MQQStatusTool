#ifndef MQ_CONNECTION_H
#define MQ_CONNECTION_H

#include <string>
#include <cstring>
#include <cmqc.h>
#include <cmqxc.h>
#include "mq_log.h"

class MQConnection {
private:
    std::string queueManager;
    std::string host;
    std::string port;
    std::string channel;
    std::string queueName;
    MQHCONN hConn;
    MQHOBJ hQueue;
    MQLog& logger;
    bool isInputQueue;

public:
    MQConnection(MQLog& log) : logger(log), hConn(MQHC_UNUSABLE_HCONN),
                               hQueue(MQHO_UNUSABLE_HOBJ), isInputQueue(true) {}

    ~MQConnection() {
        disconnect();
    }

    void setConnectionDetails(const std::string& qm, const std::string& h,
                             const std::string& p, const std::string& ch,
                             const std::string& q) {
        queueManager = qm;
        host = h;
        port = p;
        channel = ch;
        queueName = q;
    }

    bool connect() {
        logger.info("Connecting to queue manager: " + queueManager +
                     " at " + host + "(" + port + ") channel=" + channel);

        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        // Set up client connection channel definition (MQCD)
        MQCD clientConn = {MQCD_CLIENT_CONN_DEFAULT};
        clientConn.Version = MQCD_VERSION_6;

        // Set channel name
        strncpy(clientConn.ChannelName, channel.c_str(), MQ_CHANNEL_NAME_LENGTH);

        // Set connection name as "host(port)"
        std::string connName = host + "(" + port + ")";
        strncpy(clientConn.ConnectionName, connName.c_str(), MQ_CONN_NAME_LENGTH);

        // Set transport type to TCP
        clientConn.TransportType = MQXPT_TCP;

        // Set up connection options (MQCNO) for client binding
        MQCNO connOpts = {MQCNO_DEFAULT};
        connOpts.Version = MQCNO_VERSION_2;
        connOpts.Options = MQCNO_CLIENT_BINDING;
        connOpts.ClientConnPtr = &clientConn;

        MQCONNX((PMQCHAR)queueManager.c_str(), &connOpts, &hConn, &compCode, &reason);

        if (compCode != MQCC_OK) {
            logger.error("Failed to connect to " + queueManager +
                         " Reason: " + std::to_string(reason) +
                         " CompCode: " + std::to_string(compCode));
            return false;
        }

        logger.info("Connected successfully to " + queueManager);
        return true;
    }

    bool openQueue(MQLONG openOptions = MQOO_INQUIRE) {
        logger.info("Opening queue: " + queueName);

        MQOD queueDesc = {MQOD_DEFAULT};
        strncpy(queueDesc.ObjectName, queueName.c_str(), MQ_Q_NAME_LENGTH);

        MQLONG compCode = MQCC_OK;
        MQLONG reason = MQRC_NONE;

        MQOPEN(hConn, &queueDesc, openOptions, &hQueue, &compCode, &reason);

        if (compCode != MQCC_OK) {
            logger.error("Failed to open queue: " + queueName +
                         " Reason: " + std::to_string(reason));
            return false;
        }

        logger.info("Queue opened successfully");
        return true;
    }

    bool isInputOnly() const { return isInputQueue; }

    void disconnect() {
        if (hQueue != MQHO_UNUSABLE_HOBJ) {
            MQLONG compCode = MQCC_OK;
            MQLONG reason = MQRC_NONE;
            MQCLOSE(hConn, &hQueue, MQCO_NONE, &compCode, &reason);
            hQueue = MQHO_UNUSABLE_HOBJ;
        }

        if (hConn != MQHC_UNUSABLE_HCONN) {
            MQLONG compCode = MQCC_OK;
            MQLONG reason = MQRC_NONE;
            MQDISC(&hConn, &compCode, &reason);
            hConn = MQHC_UNUSABLE_HCONN;
            logger.info("Disconnected from " + queueManager);
        }
    }

    MQHCONN getHandle() const { return hConn; }
    MQHOBJ getQueueHandle() const { return hQueue; }
    std::string getQueueName() const { return queueName; }
    std::string getQueueManagerName() const { return queueManager; }
};

#endif // MQ_CONNECTION_H

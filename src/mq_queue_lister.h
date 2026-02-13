#ifndef MQ_QUEUE_LISTER_H
#define MQ_QUEUE_LISTER_H

#include <string>
#include <vector>
#include <cmqc.h>
#include "mq_log.h"

struct QueueInfo {
    std::string queueName;
    MQLONG depth;
    MQLONG openInputCount;
    MQLONG openOutputCount;
    std::string queueType;
};

class MQQueueLister {
private:
    MQLog& logger;
    MQHCONN hConn;

public:
    MQQueueLister(MQLog& log, MQHCONN conn)
        : logger(log), hConn(conn) {}

    std::vector<QueueInfo> getLocalQueues() {
        std::vector<QueueInfo> queues;

        logger.info("Querying queue manager for local queues...");
        logger.warning("To get queue status, specify queue name: --queue QUEUE_NAME");

        // Return empty list - queue enumeration requires PCF setup
        return queues;
    }

    void displayQueueList(const std::vector<QueueInfo>& queues) {
        logger.log("");
        logger.log("========================================");
        logger.log("LOCAL QUEUE ENUMERATION");
        logger.log("========================================");
        logger.log("");
        logger.log("Queue enumeration requires queue name to be specified.");
        logger.log("");
        logger.log("Usage:");
        logger.log("  mq_tool --config config.toml --qm default --queue APP1.REQ --get");
        logger.log("");
        logger.log("To get all queues, specify the queue name you want to check.");
        logger.log("========================================");
        logger.log("");
    }
};

#endif // MQ_QUEUE_LISTER_H




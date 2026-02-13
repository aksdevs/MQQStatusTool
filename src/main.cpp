#include <iostream>
#include <cstring>
#include <cmqc.h>
#include "mq_log.h"
#include "mq_configuration.h"
#include "mq_connection.h"
#include "mq_queue_status.h"
#include "mq_args.h"
#include "mq_pcf_status_inquirer.h"
#include "mq_thread_pool.h"
#include "mq_operations.h"
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>

using namespace std;

// Mutex for CSV file writing from multiple threads
static mutex csvMutex;

void generateCSVReport(const vector<PCFQueueData>& queues, const string& csvPath,
                       const string& qmName, MQLog& logger);

int main(int argc, char* argv[]) {
    CommandLineArgs args = CommandLineArgs::parse(argc, argv);

    if (args.showHelp) {
        args.printHelp(argv[0]);
        return 0;
    }

    if (args.configFile.empty()) {
        cerr << "ERROR: --config file is required" << endl;
        return 1;
    }

    if (args.queueManager.empty()) {
        cerr << "ERROR: --qm (queue manager name) is required" << endl;
        return 1;
    }

    // Load configuration
    MQConfiguration config;
    if (!config.loadFromFile(args.configFile)) {
        cerr << "ERROR: Failed to load configuration from: " << args.configFile << endl;
        return 1;
    }

    GlobalConfig globalConfig = config.getGlobalConfig();

    // Create logger
    MQLog logger(globalConfig.logPath.empty() ? "MQQStatusTool.log" : globalConfig.logPath,
                 globalConfig.logSizeMB, globalConfig.logBackups);
    logger.log("========================================");
    logger.log("IBM MQ Queue Status Tool");
    logger.log("========================================");
    logger.info("Configuration loaded successfully");

    // Load queue manager names from input file or use single QM
    vector<string> qmNamesToProcess;
    if (!args.inputFile.empty()) {
        ifstream in(args.inputFile);
        string line;
        while (getline(in, line)) {
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(remove(line.begin(), line.end(), '\n'), line.end());
            // Trim spaces
            size_t start = line.find_first_not_of(" \t");
            size_t end = line.find_last_not_of(" \t");
            if (start != string::npos) {
                line = line.substr(start, end - start + 1);
            }
            if (!line.empty() && line[0] != '#') {
                qmNamesToProcess.push_back(line);
            }
        }
        in.close();
    }

    if (qmNamesToProcess.empty()) {
        qmNamesToProcess.push_back(args.queueManager);
    }

    // Group queue managers by host to avoid duplicate connections
    map<string, vector<QMConfig>> hostGroups;
    for (const auto& qmName : qmNamesToProcess) {
        QMConfig qmCfg = config.getQueueManager(qmName);
        if (qmCfg.queueManager.empty()) {
            logger.error("Queue manager '" + qmName + "' not found in config");
            continue;
        }
        hostGroups[qmCfg.host].push_back(qmCfg);
    }

    if (hostGroups.empty()) {
        logger.error("No valid queue managers to process");
        return 1;
    }

    // Create thread pool (one thread per host, max globalConfig.maxThreads)
    int poolSize = min(globalConfig.maxThreads, (int)hostGroups.size());
    if (poolSize < 1) poolSize = 1;
    logger.info("Starting thread pool with " + to_string(poolSize) + " worker(s) for " +
                to_string(hostGroups.size()) + " host(s)");

    ThreadPool pool(poolSize);

    // Capture operation flags
    bool doStatus = args.getAllQueues;
    bool doGet = args.doGet;
    bool doPut = args.doPut;
    string targetQueue = args.queueName;

    for (auto& entry : hostGroups) {
        const string host = entry.first;
        vector<QMConfig> qms = entry.second;

        pool.enqueue([host, qms, &logger, &globalConfig, doStatus, doGet, doPut, targetQueue]() {
            logger.info("=== Thread processing host: " + host + " with " +
                        to_string(qms.size()) + " queue manager(s) ===");

            for (const auto& qmCfg : qms) {
                logger.info("Processing: " + qmCfg.queueManager + " on " + host);

                MQConnection mqConn(logger);
                mqConn.setConnectionDetails(qmCfg.queueManager, qmCfg.host,
                                           qmCfg.port, qmCfg.channel, qmCfg.queueName);

                if (!mqConn.connect()) {
                    logger.error("Failed to connect to " + qmCfg.queueManager);
                    continue;
                }

                // PUT operation
                if (doPut) {
                    string queue = targetQueue.empty() ? qmCfg.queueName : targetQueue;
                    logger.info("Putting test message to queue: " + queue);

                    string testMsg = "Test message from MQQStatusTool at " +
                                     to_string(time(nullptr));
                    MQLONG reason = MQOps::putMessage(mqConn.getHandle(),
                                                      queue.c_str(),
                                                      testMsg.c_str(),
                                                      (MQLONG)testMsg.length());
                    if (reason == MQRC_NONE) {
                        logger.info("Message put successfully to " + queue);
                    } else {
                        logger.error("Failed to put message, reason: " + to_string(reason));
                    }
                }

                // GET operation
                if (doGet) {
                    string queue = targetQueue.empty() ? qmCfg.queueName : targetQueue;
                    logger.info("Getting message from queue: " + queue);

                    unsigned char buffer[4096];
                    memset(buffer, 0, sizeof(buffer));
                    MQLONG dataLen = 0;
                    MQLONG reason = MQOps::getMessage(mqConn.getHandle(),
                                                      queue.c_str(),
                                                      buffer, sizeof(buffer),
                                                      dataLen, 5000);
                    if (reason == MQRC_NONE) {
                        logger.info("Message received (" + to_string(dataLen) + " bytes): " +
                                    string((char*)buffer, dataLen));
                    } else if (reason == MQRC_NO_MSG_AVAILABLE) {
                        logger.info("No messages available on " + queue);
                    } else {
                        logger.error("Failed to get message, reason: " + to_string(reason));
                    }
                }

                // STATUS operation (default) - Use PCF to get all local queues
                if (doStatus) {
                    MQPCFStatusInquirer inquirer(logger, mqConn.getHandle());
                    vector<PCFQueueData> queueStatuses = inquirer.inquireAllQueueStatuses();

                    if (queueStatuses.empty()) {
                        logger.warning("No queues returned from PCF for " + qmCfg.queueManager);
                    } else {
                        logger.log("");
                        logger.log("========================================");
                        logger.log("QUEUE STATUS REPORT - " + qmCfg.queueManager);
                        logger.log("========================================");
                        logger.log("");
                        logger.log("Queue Name                         | Type    | Depth | Input | Output");
                        logger.log("----------------------------------------------------------------------");

                        for (const auto& q : queueStatuses) {
                            ostringstream oss;
                            oss << left << setw(35) << q.queueName << "| "
                                << setw(8) << q.queueType << "| "
                                << right << setw(5) << q.currentDepth << " | "
                                << setw(5) << q.openInputCount << " | "
                                << setw(6) << q.openOutputCount;
                            logger.log(oss.str());
                        }

                        logger.log("----------------------------------------------------------------------");
                        logger.log("Total: " + to_string(queueStatuses.size()) + " queues");
                        logger.log("");

                        // Generate CSV if enabled
                        if (globalConfig.generateCSV) {
                            generateCSVReport(queueStatuses, globalConfig.csvPath,
                                            qmCfg.queueManager, logger);
                        }
                    }
                }

                mqConn.disconnect();
                logger.info("Completed: " + qmCfg.queueManager);
            }
        });
    }

    // Wait for all threads to complete
    pool.waitAll();
    logger.info("Thread pool shutdown complete");

    logger.log("========================================");
    logger.info("Operation completed successfully");
    return 0;
}

void generateCSVReport(const vector<PCFQueueData>& queues, const string& csvPath,
                       const string& qmName, MQLog& logger) {
    try {
        lock_guard<mutex> guard(csvMutex);

        // Check if file exists to determine if header is needed
        bool writeHeader = true;
        {
            ifstream check(csvPath);
            if (check.good() && check.peek() != ifstream::traits_type::eof()) {
                writeHeader = false;
            }
        }

        ofstream csvFile(csvPath, ios::app);
        if (!csvFile.is_open()) {
            logger.error("Could not open CSV file: " + csvPath);
            return;
        }

        if (writeHeader) {
            csvFile << "Queue_Manager,Queue_Name,Queue_Type,Current_Depth,Input_Count,Output_Count\n";
        }

        for (const auto& q : queues) {
            csvFile << qmName << "," << q.queueName << "," << q.queueType << ","
                    << q.currentDepth << "," << q.openInputCount << ","
                    << q.openOutputCount << "\n";
        }
        csvFile.close();
        logger.info("CSV data appended to: " + csvPath);
    } catch (const exception& e) {
        logger.error("Error generating CSV: " + string(e.what()));
    }
}

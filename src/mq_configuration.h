#ifndef MQ_CONFIGURATION_H
#define MQ_CONFIGURATION_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

struct QMConfig {
    std::string sectionName;     // [queuemanager.NAME] - the NAME part
    std::string queueManager;    // queue_manager = value
    std::string host;
    std::string port;
    std::string channel;
    std::string queueName;
};

struct GlobalConfig {
    std::string logPath;
    int logSizeMB;
    int logBackups;
    bool generateCSV;
    std::string csvPath;
    int maxThreads;
};

class MQConfiguration {
private:
    GlobalConfig globalConfig;
    std::vector<QMConfig> queueManagers;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::string removeQuotes(const std::string& str) {
        std::string result = str;
        if (!result.empty() && result[0] == '"') result = result.substr(1);
        if (!result.empty() && result[result.length() - 1] == '"')
            result = result.substr(0, result.length() - 1);
        return result;
    }

public:
    MQConfiguration() {
        globalConfig.logPath = "MQQStatusTool.log";
        globalConfig.logSizeMB = 10;
        globalConfig.logBackups = 5;
        globalConfig.generateCSV = true;
        globalConfig.csvPath = "queue_status.csv";
        globalConfig.maxThreads = 5;
    }

    bool loadFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "ERROR: Could not open config file: " << filePath << std::endl;
            return false;
        }

        std::string line;
        QMConfig currentQM;
        bool inGlobalSection = false;
        bool inQMSection = false;

        while (getline(file, line)) {
            line = trim(line);

            if (line.empty() || line[0] == '#' || line[0] == ';') continue;

            if (line[0] == '[' && line[line.length() - 1] == ']') {
                if (inQMSection && !currentQM.sectionName.empty()) {
                    queueManagers.push_back(currentQM);
                }
                currentQM = QMConfig();

                if (line == "[global]") {
                    inGlobalSection = true;
                    inQMSection = false;
                } else if (line.find("[queuemanager.") == 0) {
                    inGlobalSection = false;
                    inQMSection = true;
                    // Extract section name: [queuemanager.NAME] -> NAME
                    size_t start = line.find('.') + 1;
                    size_t end = line.rfind(']');
                    currentQM.sectionName = line.substr(start, end - start);
                }
                continue;
            }

            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = trim(line.substr(0, eqPos));
            std::string value = trim(line.substr(eqPos + 1));
            value = removeQuotes(value);

            if (inGlobalSection) {
                if (key == "log_file_path") globalConfig.logPath = value;
                else if (key == "log_file_size_mb") globalConfig.logSizeMB = std::stoi(value);
                else if (key == "log_backups") globalConfig.logBackups = std::stoi(value);
                else if (key == "generate_csv") globalConfig.generateCSV = (value == "true");
                else if (key == "csv_file_path") globalConfig.csvPath = value;
                else if (key == "max_threads") globalConfig.maxThreads = std::stoi(value);
            } else if (inQMSection) {
                if (key == "queue_manager") currentQM.queueManager = value;
                else if (key == "host") currentQM.host = value;
                else if (key == "port") currentQM.port = value;
                else if (key == "channel") currentQM.channel = value;
                else if (key == "queue_name") currentQM.queueName = value;
            }
        }

        if (inQMSection && !currentQM.sectionName.empty()) {
            queueManagers.push_back(currentQM);
        }

        file.close();
        return !queueManagers.empty();
    }

    QMConfig getQueueManager(const std::string& name) {
        // First try matching by section name (e.g., "default")
        for (const auto& qm : queueManagers) {
            if (qm.sectionName == name) {
                return qm;
            }
        }
        // Then try matching by queue manager name (e.g., "MQQM1")
        for (const auto& qm : queueManagers) {
            if (qm.queueManager == name) {
                return qm;
            }
        }
        return QMConfig();
    }

    GlobalConfig getGlobalConfig() const { return globalConfig; }
    const std::vector<QMConfig>& getAllQueueManagers() const { return queueManagers; }
};

#endif // MQ_CONFIGURATION_H


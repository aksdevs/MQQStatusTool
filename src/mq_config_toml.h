#ifndef MQ_CONFIG_TOML_H
#define MQ_CONFIG_TOML_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

/**
 * Global Configuration Settings
 */
struct GlobalConfig {
    string logFilePath = "MQQStatusTool.log";
    int logFileSizeMB = 10;
    int logBackups = 5;
    bool generateCSV = true;
    string csvFilePath = "queue_status.csv";
};

/**
 * Queue Manager Connection Configuration
 */
struct QMConnection {
    string queueManager;
    string host;
    string port;
    string channel;
    string queueName;
    string replyQueue;
};

/**
 * TOML Configuration Parser
 * Simple TOML parser for MQ connection details and global settings
 */
class TomlConfig {
private:
    GlobalConfig globalConfig;
    vector<QMConnection> connections;

    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    string removeQuotes(const string& str) {
        string result = str;
        if (!result.empty() && result[0] == '"') {
            result = result.substr(1);
        }
        if (!result.empty() && result[result.length() - 1] == '"') {
            result = result.substr(0, result.length() - 1);
        }
        return result;
    }

public:
    bool loadFromFile(const string& filePath) {
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "ERROR: Could not open config file: " << filePath << endl;
            return false;
        }

        string line;
        QMConnection currentConnection;
        bool inGlobalSection = false;
        bool inQMSection = false;

        while (getline(file, line)) {
            line = trim(line);

            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }

            if (line[0] == '[' && line[line.length() - 1] == ']') {
                if (inQMSection && !currentConnection.queueManager.empty()) {
                    connections.push_back(currentConnection);
                }
                currentConnection = QMConnection();

                if (line == "[global]") {
                    inGlobalSection = true;
                    inQMSection = false;
                } else if (line.find("[queuemanager.") == 0) {
                    inGlobalSection = false;
                    inQMSection = true;
                }
                continue;
            }

            size_t eqPos = line.find('=');
            if (eqPos == string::npos) continue;

            string key = trim(line.substr(0, eqPos));
            string value = trim(line.substr(eqPos + 1));
            value = removeQuotes(value);

            if (inGlobalSection) {
                if (key == "log_file_path") globalConfig.logFilePath = value;
                else if (key == "log_file_size_mb") globalConfig.logFileSizeMB = stoi(value);
                else if (key == "log_backups") globalConfig.logBackups = stoi(value);
                else if (key == "generate_csv") globalConfig.generateCSV = (value == "true");
                else if (key == "csv_file_path") globalConfig.csvFilePath = value;
            } else if (inQMSection) {
                if (key == "queue_manager") currentConnection.queueManager = value;
                else if (key == "host") currentConnection.host = value;
                else if (key == "port") currentConnection.port = value;
                else if (key == "channel") currentConnection.channel = value;
                else if (key == "queue_name") currentConnection.queueName = value;
                else if (key == "reply_queue") currentConnection.replyQueue = value;
            }
        }

        if (inQMSection && !currentConnection.queueManager.empty()) {
            connections.push_back(currentConnection);
        }

        file.close();
        return !connections.empty();
    }

    const GlobalConfig& getGlobalConfig() const {
        return globalConfig;
    }

    const vector<QMConnection>& getConnections() const {
        return connections;
    }

    QMConnection getConnection(size_t index) const {
        if (index < connections.size()) {
            return connections[index];
        }
        return QMConnection();
    }

    QMConnection getConnectionByQM(const string& qmName) const {
        for (const auto& conn : connections) {
            if (conn.queueManager == qmName) {
                return conn;
            }
        }
        return QMConnection();
    }

    size_t getConnectionCount() const {
        return connections.size();
    }
};

#endif // MQ_CONFIG_TOML_H


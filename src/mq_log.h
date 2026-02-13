#ifndef MQ_LOG_H
#define MQ_LOG_H

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>

class MQLog {
private:
    std::ofstream logFile;
    std::string logPath;
    long maxFileSize;
    int maxBackups;
    long currentSize;
    std::mutex logMutex;

    std::string getTimestamp() const {
        time_t now = time(0);
        struct tm timeinfo;
#ifdef _WIN32
        localtime_s(&timeinfo, &now);
#else
        localtime_r(&now, &timeinfo);
#endif
        std::ostringstream oss;
        oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

public:
    MQLog(const std::string& path, int sizeMB = 10, int backups = 5)
        : logPath(path), maxBackups(backups), currentSize(0) {
        maxFileSize = sizeMB * 1024 * 1024;
        logFile.open(path, std::ios::trunc);
        if (!logFile.is_open()) {
            std::cerr << "WARNING: Could not open log file: " << path << std::endl;
        }
    }

    ~MQLog() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void info(const std::string& msg) {
        log("[INFO] " + msg);
    }

    void error(const std::string& msg) {
        log("[ERROR] " + msg);
    }

    void warning(const std::string& msg) {
        log("[WARNING] " + msg);
    }

    void log(const std::string& msg) {
        std::lock_guard<std::mutex> guard(logMutex);
        std::string output = "[" + getTimestamp() + "] " + msg;
        std::cout << output << std::endl;
        std::cout.flush();

        if (logFile.is_open()) {
            logFile << output << std::endl;
            logFile.flush();
            currentSize += output.length() + 1;
        }
    }

    std::string getPath() const { return logPath; }
};

#endif // MQ_LOG_H

#ifndef MQ_LOGGER_H
#define MQ_LOGGER_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

using namespace std;

/**
 * Logger - Handles both console and file logging with rollover capability
 */
class Logger {
private:
    ofstream logFile;
    string logFilePath;
    string baseLogPath;
    long maxFileSize;        // Max size before rollover (bytes)
    int maxBackupFiles;      // Max number of backup files to keep
    long currentFileSize;

    /**
     * Get file size in bytes
     */
    long getFileSize(const string& filePath) const {
        struct stat statbuf;
        if (stat(filePath.c_str(), &statbuf) != 0) {
            return 0;
        }
        return statbuf.st_size;
    }

    /**
     * Rotate log files
     */
    void rotateLogFiles() {
        if (currentFileSize < maxFileSize) {
            return;  // No rotation needed
        }

        logFile.close();

        // Rename existing backup files
        for (int i = maxBackupFiles - 1; i > 0; --i) {
            ostringstream oldName, newName;
            oldName << baseLogPath << "." << i;
            newName << baseLogPath << "." << (i + 1);

            ifstream src(oldName.str());
            if (src.good()) {
                src.close();
                remove(oldName.str().c_str());
                rename(oldName.str().c_str(), newName.str().c_str());
            }
        }

        // Rename current log to .1
        ostringstream backupName;
        backupName << baseLogPath << ".1";
        remove(backupName.str().c_str());
        rename(baseLogPath.c_str(), backupName.str().c_str());

        // Open new log file
        logFile.open(baseLogPath, ios::app);
        currentFileSize = 0;
    }

public:
    /**
     * Initialize logger with log file path
     * maxSize: Maximum log file size in MB (default 10MB)
     * maxBackups: Number of backup files to keep (default 5)
     */
    Logger(const string& logPath, int maxSizeMB = 10, int maxBackups = 5)
        : logFilePath(logPath), baseLogPath(logPath), maxBackupFiles(maxBackups),
          currentFileSize(0) {
        maxFileSize = maxSizeMB * 1024 * 1024;
        logFile.open(logPath, ios::app);
        if (!logFile.is_open()) {
            cerr << "ERROR: Could not open log file: " << logPath << endl;
        } else {
            currentFileSize = getFileSize(logPath);
        }
    }

    /**
     * Destructor - close log file
     */
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    /**
     * Get current timestamp with milliseconds
     */
    string getTimestamp() const {
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        ostringstream oss;
        oss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    /**
     * Get current timestamp with milliseconds (more detailed)
     */
    string getDetailedTimestamp() const {
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        ostringstream oss;
        oss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S.") << (now % 1000);
        return oss.str();
    }

    /**
     * Log a message (console + file)
     */
    void log(const string& message, bool addTimestamp = true) {
        string output = message;
        if (addTimestamp) {
            output = "[" + getTimestamp() + "] " + message;
        }

        // Output to console
        cout << output << endl;

        // Output to file with rollover check
        if (logFile.is_open()) {
            logFile << output << endl;
            logFile.flush();
            currentFileSize += output.length() + 1;  // +1 for newline

            // Check if rotation is needed
            if (currentFileSize >= maxFileSize) {
                rotateLogFiles();
            }
        }
    }

    /**
     * Log an error message
     */
    void error(const string& message) {
        string output = "[ERROR] " + message;
        log(output, true);
    }

    /**
     * Log a warning message
     */
    void warning(const string& message) {
        string output = "[WARNING] " + message;
        log(output, true);
    }

    /**
     * Log info message
     */
    void info(const string& message) {
        string output = "[INFO] " + message;
        log(output, true);
    }

    /**
     * Log separator line
     */
    void separator(char ch = '=', int width = 60) {
        string line(width, ch);
        log(line, false);
    }

    /**
     * Log section header
     */
    void header(const string& title) {
        separator();
        log(title, false);
        separator();
    }

    /**
     * Get log file path
     */
    string getLogFilePath() const {
        return logFilePath;
    }

    /**
     * Check if log file is open
     */
    bool isOpen() const {
        return logFile.is_open();
    }
};

#endif // MQ_LOGGER_H


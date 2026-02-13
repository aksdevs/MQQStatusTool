#ifndef MQ_TEXT_FILE_PROCESSOR_H
#define MQ_TEXT_FILE_PROCESSOR_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

/**
 * Text File Processor - Reads queue manager names from text file
 */
namespace TextFileProcessor {

    /**
     * Read queue manager names from text file
     * Each line should contain a queue manager name that exists in config
     */
    inline vector<string> readQueueManagersFromFile(const string& filePath) {
        vector<string> queueManagers;
        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "ERROR: Could not open queue manager list file: " << filePath << endl;
            return queueManagers;
        }

        string line;
        while (getline(file, line)) {
            // Trim whitespace
            size_t start = line.find_first_not_of(" \t\r\n");
            size_t end = line.find_last_not_of(" \t\r\n");

            if (start == string::npos) continue;  // Empty line
            if (line[0] == '#' || line[0] == ';') continue;  // Comment line

            string trimmedLine = line.substr(start, (end - start + 1));
            if (!trimmedLine.empty()) {
                queueManagers.push_back(trimmedLine);
            }
        }

        file.close();
        return queueManagers;
    }

    /**
     * Validate queue manager exists in config
     * Returns true if QM is found in config
     */
    inline bool validateQueueManager(const string& qmName, const vector<string>& configuredQMs) {
        for (const auto& qm : configuredQMs) {
            if (qm == qmName) {
                return true;
            }
        }
        return false;
    }
}

#endif // MQ_TEXT_FILE_PROCESSOR_H


#ifndef MQ_CSV_WRITER_H
#define MQ_CSV_WRITER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

/**
 * Queue Handle Information
 */
struct QueueHandle {
    string queueName;
    string queueManager;
    int handleCount = 0;
    int ipProcs = 0;      // Input processes (readers)
    int opProcs = 0;      // Output processes (writers)
    string connectionName;
    string channelName;
    string applicationTag;
    int processId = 0;
    int depth = 0;
    string handleType;    // "Reader" or "Writer"
};

/**
 * CSV Writer - Writes queue status to CSV file
 */
namespace CSVWriter {

    /**
     * Write queue handles to CSV file
     */
    inline bool writeQueueStatus(const string& csvPath, const vector<QueueHandle>& handles) {
        if (handles.empty()) {
            return true;  // Nothing to write
        }

        ofstream csvFile(csvPath);
        if (!csvFile.is_open()) {
            cerr << "ERROR: Could not open CSV file for writing: " << csvPath << endl;
            return false;
        }

        // Write CSV header
        csvFile << "Queue_Manager,Queue_Name,Handle_Type,Process_ID,IP_Procs,OP_Procs,"
                << "Connection_Name,Channel_Name,Application_Tag,Queue_Depth,Handle_Count\n";

        // Write data rows
        for (const auto& handle : handles) {
            csvFile << handle.queueManager << ","
                    << handle.queueName << ","
                    << handle.handleType << ","
                    << handle.processId << ","
                    << handle.ipProcs << ","
                    << handle.opProcs << ","
                    << handle.connectionName << ","
                    << handle.channelName << ","
                    << handle.applicationTag << ","
                    << handle.depth << ","
                    << handle.handleCount << "\n";
        }

        csvFile.close();
        return true;
    }

    /**
     * Append queue handles to CSV file
     */
    inline bool appendQueueStatus(const string& csvPath, const vector<QueueHandle>& handles) {
        if (handles.empty()) {
            return true;
        }

        ofstream csvFile(csvPath, ios::app);
        if (!csvFile.is_open()) {
            cerr << "ERROR: Could not open CSV file for appending: " << csvPath << endl;
            return false;
        }

        for (const auto& handle : handles) {
            csvFile << handle.queueManager << ","
                    << handle.queueName << ","
                    << handle.handleType << ","
                    << handle.processId << ","
                    << handle.ipProcs << ","
                    << handle.opProcs << ","
                    << handle.connectionName << ","
                    << handle.channelName << ","
                    << handle.applicationTag << ","
                    << handle.depth << ","
                    << handle.handleCount << "\n";
        }

        csvFile.close();
        return true;
    }
}

#endif // MQ_CSV_WRITER_H


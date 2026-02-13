#ifndef MQ_UTILITIES_H
#define MQ_UTILITIES_H

#include <iostream>
#include <iomanip>
#include <cmqc.h>

using namespace std;

/**
 * MQ Utilities - Helper functions for MQ operations
 */

namespace MQUtil {

    /**
     * Display human-readable error messages for MQ reason codes
     */
    inline void displayError(const char* context, MQLONG reason)
    {
        cerr << "ERROR in " << context << ": Reason Code = " << reason << " (0x"
             << hex << reason << dec << ")" << endl;

        switch(reason) {
        case MQRC_NONE:
            cerr << "  → No error" << endl;
            break;
        case MQRC_NOT_CONNECTED:
            cerr << "  → Not connected to queue manager" << endl;
            break;
        case MQRC_GET_INHIBITED:
            cerr << "  → Get operations are inhibited on this queue" << endl;
            break;
        case MQRC_NO_MSG_AVAILABLE:
            cerr << "  → No messages available on queue" << endl;
            break;
        default:
            cerr << "  → See IBM MQ documentation for this reason code" << endl;
            break;
        }
    }

    /**
     * Convert byte array to hexadecimal string representation
     */
    inline string bytesToHex(const unsigned char* data, int length)
    {
        ostringstream oss;
        for (int i = 0; i < length; i++) {
            oss << hex << setw(2) << setfill('0') << (int)data[i];
        }
        return oss.str();
    }

    /**
     * Display message details in a formatted way
     */
    inline void displayMessage(int messageNum, const MQMD& msgDesc,
                               const unsigned char* msgBuffer, MQLONG dataLength,
                               int displayLimit)
    {
        cout << "\n--- Message #" << messageNum << " ---" << endl;
        cout << "  Message ID:    " << bytesToHex(msgDesc.MsgId, MQ_MSG_ID_LENGTH) << endl;
        cout << "  Correlation ID:" << bytesToHex(msgDesc.CorrelId, MQ_CORREL_ID_LENGTH) << endl;
        cout << "  Length:        " << dataLength << " bytes" << endl;
        cout << "  Type:          " << msgDesc.MsgType << endl;
        cout << "  Priority:      " << msgDesc.Priority << endl;
        cout << "  Persistence:   " << msgDesc.Persistence << endl;
        cout << "  Content:       ";

        for (int i = 0; i < dataLength && i < displayLimit; i++) {
            if (isprint(msgBuffer[i])) {
                cout << (char)msgBuffer[i];
            } else {
                cout << ".";
            }
        }

        if (dataLength > displayLimit) {
            cout << " ... (truncated - full message is " << dataLength << " bytes)";
        }
        cout << endl;
    }

    /**
     * Separator line for output
     */
    inline void printSeparator(char ch = '=', int width = 60)
    {
        for (int i = 0; i < width; i++) {
            cout << ch;
        }
        cout << endl;
    }

    /**
     * Display section header
     */
    inline void printHeader(const string& title)
    {
        cout << endl;
        printSeparator('=', 60);
        cout << "  " << title << endl;
        printSeparator('=', 60);
    }

    /**
     * Get completion code description
     */
    inline string getCompCodeDescription(MQLONG compCode)
    {
        switch(compCode) {
            case MQCC_OK:
                return "OK (Success)";
            case MQCC_WARNING:
                return "WARNING";
            case MQCC_FAILED:
                return "FAILED";
            default:
                return "UNKNOWN";
        }
    }

} // namespace MQUtil

#endif // MQ_UTILITIES_H


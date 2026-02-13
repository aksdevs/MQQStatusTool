#ifndef MQ_ARGS_H
#define MQ_ARGS_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

/**
 * Command-line Arguments Parser
 */
struct CommandLineArgs {
    bool doGet = false;          // Get queue messages
    bool doPut = false;          // Put test message
    bool showHelp = false;       // Show help
    bool getAllQueues = true;    // Get status of all local queues (default)
    string configFile = "";      // Path to TOML config file
    string queueManager = "";    // Queue manager name (required, must be in TOML)
    string queueName = "";       // Queue name (optional, required for GET/PUT)
    string inputFile = "";       // Text file with QM names for batch processing
    int logSizeMB = 10;         // Log file size in MB
    int maxLogBackups = 5;      // Max number of log backups

    /**
     * Display help message
     */
    void printHelp(const char* programName) const {
        cout << "\n=== IBM MQ Queue Status Tool ===" << endl;
        cout << "\nUsage: " << programName << " --config <file> --qm <qm_name> [OPTIONS]" << endl;
        cout << "\nRequired Arguments:" << endl;
        cout << "  --config <file>       Path to TOML configuration file (must contain QM)" << endl;
        cout << "  --qm <name>           Queue manager name (must exist in TOML config)" << endl;
        cout << "\nOperation Flags:" << endl;
        cout << "  --status              Get status of all local queues (default)" << endl;
        cout << "  --queue <name>        Specify queue name for GET/PUT operations" << endl;
        cout << "  --get                 Get messages from specified queue" << endl;
        cout << "  --put                 Put test message to specified queue" << endl;
        cout << "\nOptional Arguments:" << endl;
        cout << "  --input-file <file>   Text file with queue manager names (batch mode)" << endl;
        cout << "  --log-size <MB>       Max log file size in MB (default 10)" << endl;
        cout << "  --log-backups <num>   Number of log backups to keep (default 5)" << endl;
        cout << "  --help                Show this help message" << endl;
        cout << "\nExamples:" << endl;
        cout << "  " << programName << " --config config.toml --qm default --status" << endl;
        cout << "  " << programName << " --config config.toml --qm default --queue APP1.REQ --get" << endl;
        cout << "  " << programName << " --config config.toml --qm default --queue APP1.REQ --put" << endl;
        cout << "\n";
    }

    /**
     * Parse command-line arguments
     */
    static CommandLineArgs parse(int argc, char* argv[]) {
        CommandLineArgs args;

        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];

            if (arg == "--help" || arg == "-h") {
                args.showHelp = true;
            }
            else if (arg == "--config") {
                if (i + 1 < argc) {
                    args.configFile = argv[++i];
                }
            }
            else if (arg == "--status") {
                args.getAllQueues = true;
                args.doGet = false;
                args.doPut = false;
            }
            else if (arg == "--get") {
                args.doGet = true;
                args.doPut = false;
                args.getAllQueues = false;
            }
            else if (arg == "--put") {
                args.doPut = true;
                args.doGet = false;
                args.getAllQueues = false;
            }
            else if (arg == "--qm") {
                if (i + 1 < argc) {
                    args.queueManager = argv[++i];
                }
            }
            else if (arg == "--queue") {
                if (i + 1 < argc) {
                    args.queueName = argv[++i];
                }
            }
            else if (arg == "--input-file") {
                if (i + 1 < argc) {
                    args.inputFile = argv[++i];
                }
            }
            else if (arg == "--log-size") {
                if (i + 1 < argc) {
                    args.logSizeMB = stoi(argv[++i]);
                }
            }
            else if (arg == "--log-backups") {
                if (i + 1 < argc) {
                    args.maxLogBackups = stoi(argv[++i]);
                }
            }
        }

        // Default to status if no operation specified
        if (!args.doGet && !args.doPut && !args.getAllQueues) {
            args.getAllQueues = true;
        }

        return args;
    }
};

#endif // MQ_ARGS_H


MQQStatusTool - IBM MQ Queue Status Monitoring Utility

A professional C++ utility for monitoring IBM MQ queue status, retrieving and 
sending messages, and batch processing multiple queue managers.

Author: Atul Kumar
Primary Maintainer: Atul Kumar
License: Apache License 2.0

OVERVIEW

MQQStatusTool is a command-line application that connects to IBM MQ queue 
managers and provides the following functionality:

- Retrieve messages from specified queues
- Send test messages to queues
- Display detailed queue status information
- List all local queues on a queue manager
- Batch process multiple queue managers
- Generate CSV reports (optional)
- Maintain detailed operation logs with timestamps

REQUIREMENTS

- IBM MQ Client 9.0 or higher
- CMake 3.20 or higher
- GCC 5.0+ (Windows: MinGW GCC) or Clang 3.8+
- Make or MinGW Make

CONFIGURATION

The application requires a TOML configuration file (config.toml) that defines:

Global Settings:
  log_file_path       Path to application log file
  log_file_size_mb    Maximum log file size before rotation (default: 10)
  log_backups         Number of backup log files to keep (default: 5)
  generate_csv        Enable/disable CSV export (true/false)
  csv_file_path       Path for CSV output file

Queue Manager Configuration:
  Each queue manager is defined in a section like [queuemanager.name]
  
  Required parameters:
    queue_manager     Queue manager name
    host             Host or IP address
    port             Connection port
    channel          Server connection channel name
    queue_name       Default queue name (can be overridden at runtime)

Example config.toml:

  [global]
  log_file_path = "C:/logs/MQQStatusTool.log"
  log_file_size_mb = 10
  log_backups = 5
  generate_csv = true
  csv_file_path = "C:/output/queue_status.csv"

  [queuemanager.default]
  queue_manager = "MQQM1"
  host = "127.0.0.1"
  port = "5200"
  channel = "APP1.SVRCONN"
  queue_name = "APP1.REQ"

BUILDING

Windows Build:
  .\build_run.ps1 -Action build

Linux Build:
  ./build_run.sh build

The build process generates the executable in the build/ directory.

RUNNING THE APPLICATION

The application requires three mandatory parameters:

  --config FILE       Path to TOML configuration file
  --qm NAME          Queue manager name (must be defined in config.toml)
  --queue NAME       Queue name to operate on

Optional parameters:

  --get              Get queue status and retrieve messages (default operation)
  --put              Send test message to specified queue
  --log-size MB      Override log file size limit
  --log-backups NUM  Override number of log backups
  --help             Display help information

USAGE EXAMPLES

Get status of a specific queue:

  Windows: .\build\MQQStatusTool.exe --config config.toml --qm default --queue APP1.REQ --get
  Linux:   ./build/MQQStatusTool --config config.toml --qm default --queue APP1.REQ --get

Send test message to a queue:

  Windows: .\build\MQQStatusTool.exe --config config.toml --qm default --queue APP1.REQ --put
  Linux:   ./build/MQQStatusTool --config config.toml --qm default --queue APP1.REQ --put

List all local queues on a queue manager:

  Windows: .\build\MQQStatusTool.exe --config config.toml --qm default --queue "*" --get
  Linux:   ./build/MQQStatusTool --config config.toml --qm default --queue "*" --get

Process multiple queue managers from input file:

  Windows: .\get_all_queues.ps1
  Linux:   ./get_all_queues.sh

The input file (qm_list.txt) should contain one queue manager name per line.

UNIFIED BUILD AND RUN SCRIPTS

Windows PowerShell Script (build_run.ps1):

  .\build_run.ps1 -Action build         Build the project
  .\build_run.ps1 -Action run           Run with default parameters
  .\build_run.ps1 -Action rebuild       Clean and rebuild
  .\build_run.ps1 -Action clean         Remove build artifacts

Parameters:
  -Qm QUEUE_MANAGER                Queue manager name
  -Queue QUEUE_NAME                Queue name
  -Op get|put                      Operation type
  -Config FILE                     Configuration file path

Linux Bash Script (build_run.sh):

  ./build_run.sh build                  Build the project
  ./build_run.sh run                    Run with default parameters
  ./build_run.sh rebuild                Clean and rebuild
  ./build_run.sh clean                  Remove build artifacts

Parameters:
  --qm QUEUE_MANAGER                Queue manager name
  --queue QUEUE_NAME                Queue name
  --op get|put                      Operation type
  --config FILE                     Configuration file path
  --debug                           Build with debug symbols

OPERATIONS

GET Operation:
  Connects to the queue manager and specified queue, then retrieves all 
  available messages. Displays queue status information including current 
  queue depth, process handles, and connection details. Logs all operations 
  with timestamps.

PUT Operation:
  Connects to the queue manager and specified queue, then sends a test 
  message. Confirms successful message delivery and logs the operation.

LIST Operation:
  Use queue name "*" to list all local queues on the queue manager. 
  Displays queue depth and handle information for each queue.

QUEUE INFORMATION DISPLAYED

When retrieving queue status, the following information is provided:

  Queue Name              Name of the queue
  Current Queue Depth    Number of messages in the queue
  Queue Status           Active or Inactive status
  Application Tag        Associated application identifier
  Connection Name        Connection details
  Channel Name          Communication channel name
  User ID               User associated with operation
  Process ID            Process identifier
  Input Process Count   Number of processes reading from queue
  Output Process Count  Number of processes writing to queue
  Open Input Count      Number of open handles for input
  Open Output Count     Number of open handles for output

LOGGING

All operations are logged to the file specified in config.toml. Log entries 
include timestamps in the format YYYY-MM-DD HH:MM:SS. Log files automatically 
rotate when the configured size limit is exceeded.

Log file naming convention:
  MQQStatusTool.log       Current log file
  MQQStatusTool.log.1     Previous log file
  MQQStatusTool.log.2     Older log file
  (up to configured backup count)

Example log entries:
  [2026-02-11 22:20:18] [INFO] Configuration loaded from config.toml
  [2026-02-11 22:20:18] [INFO] Connecting to queue manager MQQM1
  [2026-02-11 22:20:18] [INFO] Successfully connected to queue manager
  [2026-02-11 22:20:18] [INFO] Message put successfully
  [2026-02-11 22:20:19] [WARNING] Queue has 1 pending messages

CSV EXPORT

When enabled in config.toml, the application can generate CSV files containing 
queue status information. This feature is useful for reporting and analysis.

CSV columns include:
  Queue_Manager        Queue manager name
  Queue_Name          Queue name
  Handle_Type         Reader or Writer designation
  Process_ID          Process identifier
  IP_Procs            Input process count
  OP_Procs            Output process count
  Connection_Name     Connection details
  Channel_Name        Channel name
  Application_Tag     Application identifier
  Queue_Depth         Number of messages
  Handle_Count        Number of handles

BATCH PROCESSING

For processing multiple queue managers, use the batch processing scripts:

Windows:
  .\get_all_queues.ps1

Linux:
  ./get_all_queues.sh

These scripts:
  1. Read queue manager names from qm_list.txt
  2. Validate each queue manager exists in config.toml
  3. Connect to each queue manager sequentially
  4. List all local queues for each manager
  5. Generate logs and optional CSV reports
  6. Provide summary of results

Edit qm_list.txt to specify queue managers to process:
  MQQM1
  PROD_QM
  DEV_QM

TROUBLESHOOTING

Cannot connect to queue manager:
  - Verify the queue manager is running
  - Check host and port values in config.toml
  - Verify channel name is correct
  - Ensure user has proper permissions for the queue manager
  - Check network connectivity to the host

Queue manager not found in configuration:
  - Ensure [queuemanager.NAME] section exists in config.toml
  - Queue manager name in command line must exactly match the config section
  - Verify TOML syntax is correct

Cannot open queue:
  - Verify queue name is correct
  - Ensure queue exists on the queue manager
  - Verify user has permission to access the queue
  - Check queue is not defined as system queue without proper access

Build failures:
  - Ensure CMake is installed and in PATH
  - Verify GCC/MinGW is installed and accessible
  - Check IBM MQ development headers are installed
  - Review CMakeLists.txt for required paths

No log output:
  - Verify log file path in config.toml is writable
  - Check directory exists or can be created
  - Ensure proper file system permissions

ARCHITECTURE

The application is organized into focused classes:

MQLog:
  Handles all logging operations including timestamps and file rotation.
  Provides methods for info(), error(), and warning() level logging.

MQConfiguration:
  Parses TOML configuration files and provides access to settings.
  Manages multiple queue manager configurations.
  Handles global application settings.

MQConnection:
  Manages queue manager connections and lifecycle.
  Handles queue open/close operations.
  Provides handles for message operations.

MQQueueLister:
  Lists all local queues on a queue manager.
  Retrieves queue status information.
  Formats queue information for display.

Main Application (main.cpp):
  Orchestrates the overall workflow.
  Handles command-line arguments and validation.
  Coordinates between classes to execute operations.

PLATFORM SUPPORT

Windows:
  - Tested on Windows 10 and Windows 11
  - Uses PowerShell scripts (build_run.ps1, get_all_queues.ps1)
  - Compiled with MinGW GCC

Linux/Unix:
  - Tested on Ubuntu 18.04+, CentOS 7+, RHEL 7+
  - Uses Bash scripts (build_run.sh, get_all_queues.sh)
  - Compiled with GCC or Clang

macOS:
  - Bash scripts compatible
  - Requires Xcode Command Line Tools or GCC

QUICK START

1. Install Prerequisites:
   - IBM MQ Client
   - CMake
   - GCC/MinGW or Clang

2. Configure Queue Managers:
   Edit config.toml and add your queue manager details

3. Build Project:
   Windows: .\build_run.ps1 -Action build
   Linux:   ./build_run.sh build

4. Run Application:
   .\build\MQQStatusTool.exe --config config.toml --qm default --queue APP1.REQ --get

5. Check Results:
   - Console output displays immediate results
   - MQQStatusTool.log contains detailed operation log
   - Optional CSV file contains status report

SUPPORT AND ISSUES

For issues or questions:

1. Review MQQStatusTool.log for error messages and detailed information
2. Verify configuration in config.toml is correct
3. Ensure queue manager is accessible and running
4. Check user permissions for queue access
5. Review README.md for usage examples
6. Verify IBM MQ Client installation and version compatibility

LICENSE

This software is provided under the Apache License 2.0. See the LICENSE 
file in the root directory for the complete terms and conditions.

Copyright 2026 Atul Kumar

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

The author and community assume no liability for any damages or issues 
arising from use of this software.

Author: Atul Kumar
Primary Maintainer: Atul Kumar


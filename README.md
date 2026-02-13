# MQQStatusTool

IBM MQ Queue Status Monitoring and Message Management Utility

**License:** Apache License 2.0  
**Author:** Atul Kumar

---

## Overview

MQQStatusTool is a professional C++ command-line utility for monitoring IBM MQ queue managers, managing messages, and analyzing queue status across multiple systems. Built with multi-threaded architecture, it provides comprehensive queue status reporting with support for batch processing.

### Key Features

- **Message Operations:** Retrieve messages from queues and send test messages
- **Queue Status Monitoring:** Display comprehensive status information for all local queues
- **Process Analysis:** Identify reader and writer processes by PID with segregated reporting
- **Connection Details:** Track connections, users, channels, and process identifiers
- **Multi-threaded Processing:** Configurable thread pool for concurrent queue manager connections
- **CSV Export:** Optional report generation with queue manager name and timestamps
- **Batch Processing:** Process multiple queue managers from input files
- **Advanced Logging:** Timestamp-based logging with automatic file rotation
- **Dynamic Queues:** Auto-created and cleaned PCF response queues

---

## Requirements

| Component | Minimum Version | Notes |
|-----------|-----------------|-------|
| IBM MQ Client | 9.0+ | C/C++ development libraries required |
| CMake | 3.20+ | Build system |
| GCC/Clang | 5.0+ | C++11 standard support required |
| Make/MinGW Make | Latest | Build tool |
| Operating System | Windows/Linux/macOS | 64-bit architecture recommended |

---

## Directory Setup

Before running the application, ensure that the directories specified in your `config.toml` file exist or can be created by the application:

### Required Directories

| Directory | Purpose | Config Parameter | Notes |
|-----------|---------|------------------|-------|
| Log Directory | Application logs with timestamps | `log_file_path` | Must be writable. Application attempts to create if missing. |
| Output Directory | CSV reports | `csv_file_path` | Must be writable. Application attempts to create if missing. |

### Example Directory Structure

```
MQQStatusTool/
├── logs/                          (configured in log_file_path)
│   ├── MQQStatusTool.log         (current log)
│   ├── MQQStatusTool.log.1       (rotated backup)
│   └── MQQStatusTool.log.2       (rotated backup)
├── output/                        (configured in csv_file_path)
│   ├── queue_status_MQQM1_20260212_214734.csv
│   ├── queue_status_PROD_QM_20260212_215045.csv
│   └── queue_status_DEV_QM_20260212_215200.csv
├── config.toml
├── qm_list.txt
├── build.ps1
├── build.sh
├── run.ps1
├── run.sh
└── src/
    └── main.cpp
```

### Sample Logs and Output

The workspace includes sample logs and output files for reference:

- **Sample Log:** `logs/MQQStatusTool_*.log` - Contains timestamped operations log entries with INFO, WARNING, and ERROR levels
- **Sample Output:** `output/queue_status_*.csv` - Contains queue status reports with queue manager name and timestamps

Review these samples to understand the expected output format and logging level details.

---

## Configuration

Configuration is managed through a TOML file (default: `config.toml`). This file defines all queue managers and global settings.

### Global Settings

```toml
[global]
log_file_path = "logs/MQQStatusTool.log"
log_file_size_mb = 10
log_backups = 5
generate_csv = true
csv_file_path = "output/queue_status.csv"
max_threads = 5
```

| Parameter | Description | Default |
|-----------|-------------|---------|
| `log_file_path` | Directory and filename for application log | logs/MQQStatusTool.log |
| `log_file_size_mb` | Maximum log file size before rotation | 10 |
| `log_backups` | Number of rotated backup logs to maintain | 5 |
| `generate_csv` | Enable CSV report generation | true |
| `csv_file_path` | Output path for CSV reports | output/queue_status.csv |
| `max_threads` | Maximum concurrent threads for processing | 5 |

### Queue Manager Configuration

Each queue manager requires a dedicated section in the TOML file:

```toml
[queuemanager.MQQM1]
queue_manager = "MQQM1"
host = "127.0.0.1"
port = 5200
channel = "APP1.SVRCONN"
queue_name = "APP1.REQ"
```

| Parameter | Required | Description |
|-----------|----------|-------------|
| `queue_manager` | Yes | Queue manager name |
| `host` | Yes | Hostname or IP address |
| `port` | Yes | Connection port (typically 1414 for production, 5200 for testing) |
| `channel` | Yes | Server connection channel name |
| `queue_name` | No | Default queue (can be overridden at runtime) |

### Example Configuration File

```toml
[global]
log_file_path = "logs/MQQStatusTool.log"
log_file_size_mb = 10
log_backups = 5
generate_csv = true
csv_file_path = "output/queue_status.csv"
max_threads = 5

[queuemanager.MQQM1]
queue_manager = "MQQM1"
host = "127.0.0.1"
port = 5200
channel = "APP1.SVRCONN"
queue_name = "APP1.REQ"

[queuemanager.PROD_QM]
queue_manager = "PROD_QM"
host = "mq-server.example.com"
port = 1414
channel = "PROD.SVRCONN"
queue_name = "PRODUCTION.REQUEST"
```

---

## Building

### Windows (PowerShell)

```powershell
# Build the project
.\build.ps1

# Rebuild from scratch
.\build.ps1 -Clean

# Build with debug symbols
.\build.ps1 -Debug
```

### Linux/Unix (Bash)

```bash
# Build the project
./build.sh

# Rebuild from scratch
./build.sh --clean

# Build with debug symbols
./build.sh --debug
```

The build process compiles the source code and generates the executable in the `build/` directory.

---

## Running the Application

### Windows (PowerShell)

```powershell
.\run.ps1 -Config config.toml -Qm MQQM1 -Queue APP1.REQ -Op get
```

### Linux/Unix (Bash)

```bash
./run.sh --config config.toml --qm MQQM1 --queue APP1.REQ --op get
```

### Command-Line Parameters

#### Required Parameters

| Parameter | Short | Description |
|-----------|-------|-------------|
| `--config` | `-c` | Path to TOML configuration file |
| `--qm` | `-q` | Queue manager name (must exist in config.toml) |

#### Optional Parameters

| Parameter | Short | Description |
|-----------|-------|-------------|
| `--queue` | `-Q` | Queue name to operate on (default: from config) |
| `--op` | `-o` | Operation: `get`, `put`, or `status` (default: status) |
| `--help` | `-h` | Display help information |

---

## Usage Examples

### Get Queue Status (Default)

Get status of all local queues on MQQM1:

```powershell
# Windows
.\run.ps1 -Config config.toml -Qm MQQM1

# Linux
./run.sh --config config.toml --qm MQQM1
```

### Retrieve Message from Specific Queue

```powershell
# Windows
.\run.ps1 -Config config.toml -Qm MQQM1 -Queue APP1.REQ -Op get

# Linux
./run.sh --config config.toml --qm MQQM1 --queue APP1.REQ --op get
```

### Send Test Message to Queue

```powershell
# Windows
.\run.ps1 -Config config.toml -Qm MQQM1 -Queue APP1.REQ -Op put

# Linux
./run.sh --config config.toml --qm MQQM1 --queue APP1.REQ --op put
```

### Batch Processing Multiple Queue Managers

Process all queue managers listed in `qm_list.txt`:

```powershell
# Windows
.\run.ps1 -Config config.toml -BatchFile qm_list.txt

# Linux
./run.sh --config config.toml --batch-file qm_list.txt
```

The batch file should contain one queue manager name per line:

```
MQQM1
PROD_QM
DEV_QM
```

---

## Build and Run Scripts

### Windows PowerShell Scripts

**build.ps1** - Comprehensive build management

```powershell
.\build.ps1                    # Standard build
.\build.ps1 -Clean             # Clean and rebuild
.\build.ps1 -Debug             # Build with debug symbols
```

**run.ps1** - Application execution

```powershell
.\run.ps1 -Config config.toml -Qm MQQM1
.\run.ps1 -Config config.toml -Qm MQQM1 -Queue APP1.REQ -Op get
.\run.ps1 -Config config.toml -Qm MQQM1 -Queue APP1.REQ -Op put
.\run.ps1 -Config config.toml -BatchFile qm_list.txt
```

### Linux/Unix Bash Scripts

**build.sh** - Comprehensive build management

```bash
./build.sh                     # Standard build
./build.sh --clean             # Clean and rebuild
./build.sh --debug             # Build with debug symbols
```

**run.sh** - Application execution

```bash
./run.sh --config config.toml --qm MQQM1
./run.sh --config config.toml --qm MQQM1 --queue APP1.REQ --op get
./run.sh --config config.toml --qm MQQM1 --queue APP1.REQ --op put
./run.sh --config config.toml --batch-file qm_list.txt
```

---

## Operations

### GET Operation

Retrieve messages from a specified queue and display queue status:

- Connects to the specified queue manager and queue
- Retrieves all available messages
- Displays queue information including depth and handles
- Identifies active reader/writer processes by PID
- Logs all operations with timestamps
- Generates CSV report (if enabled)

### PUT Operation

Send a test message to a specified queue:

- Connects to the specified queue manager and queue
- Creates and sends a test message
- Confirms successful message delivery
- Logs the operation with timestamp
- Updates CSV report (if enabled)

### STATUS Operation (Default)

Display comprehensive status of all local queues:

- Queries all local queues on the queue manager
- Uses PCF INQUIRE_Q_STATUS for detailed status
- Displays queue depth, handles, and process information
- Identifies reader/writer processes by operation type
- Segregates multiple processes by PID
- Generates CSV report with all queue details
- Logs complete status snapshot

---

## Queue Information Displayed

The utility displays comprehensive queue status information:

| Field | Description |
|-------|-------------|
| Queue Name | Name of the queue |
| Queue Manager | Associated queue manager |
| Queue Depth | Current number of messages in the queue |
| Queue Type | Queue classification (LOCAL, REMOTE, etc.) |
| Handle Type | Process operation type (READ, WRITE, READ/WRITE) |
| Process ID | Operating system process identifier |
| Input Procs | Number of processes reading from the queue |
| Output Procs | Number of processes writing to the queue |
| Open Input Count | Number of open handles for input operations |
| Open Output Count | Number of open handles for output operations |
| Connection Name | Connection identifier or N/A if not available |
| Channel Name | Communication channel name or N/A |
| User ID | User executing the operation or N/A |
| Application Tag | Associated application identifier or N/A |
| Timestamp | Operation timestamp (YYYY-MM-DD HH:MM:SS) |

---

## Logging

All operations are automatically logged to the file specified in `config.toml` under the `[global]` section with the `log_file_path` parameter.

### Log Features

- **Timestamps:** All entries include timestamps in the format `YYYY-MM-DD HH:MM:SS`
- **Log Levels:** INFO, WARNING, ERROR categorization
- **Automatic Rotation:** Logs rotate when the configured size limit is exceeded
- **Backup Management:** Configurable number of backup logs retained
- **Directory Creation:** Log directories are automatically created if they don't exist

### Log File Naming

Current log file and rotated backups follow this naming convention:

```
logs/
  MQQStatusTool.log      (Current log file)
  MQQStatusTool.log.1    (Previous log file)
  MQQStatusTool.log.2    (Older log file)
  MQQStatusTool.log.N    (Up to configured backup count)
```

### Log Format Example

```
[2026-02-12 21:47:32] [INFO] Configuration loaded from config.toml
[2026-02-12 21:47:32] [INFO] Connecting to queue manager MQQM1
[2026-02-12 21:47:33] [INFO] Successfully connected to queue manager
[2026-02-12 21:47:33] [INFO] Getting status of all local queues
[2026-02-12 21:47:33] [INFO] Found 5 local queues
[2026-02-12 21:47:34] [INFO] CSV report generated: output/queue_status_MQQM1_20260212_214734.csv
[2026-02-12 21:47:34] [INFO] Disconnecting from queue manager
```

---

## CSV Export

When enabled in `config.toml`, the utility generates CSV reports containing queue status details. CSV export is optional and controlled by the `generate_csv` setting.

### CSV Features

- **Conditional Generation:** Enable or disable via `generate_csv` in config
- **Timestamped Reports:** Each file includes queue manager name and timestamp
- **Multi-Handle Support:** Separate entries for each process (reader/writer)
- **Complete Details:** Includes all queue status fields
- **Queue Manager Tracking:** Queue manager name prepended to filename

### CSV File Naming

```
output/
  queue_status_MQQM1_20260212_214734.csv
  queue_status_PROD_QM_20260212_215045.csv
```

### CSV Columns

| Column | Description |
|--------|-------------|
| Queue_Manager | Queue manager name |
| Queue_Name | Queue name |
| Queue_Type | Queue type (LOCAL, REMOTE, etc.) |
| Queue_Depth | Current message count |
| Handle_Type | Process operation type (READ, WRITE, READ/WRITE) |
| Process_ID | Operating system process identifier |
| IP_Procs | Input process count |
| OP_Procs | Output process count |
| Open_Input_Count | Open handles for input |
| Open_Output_Count | Open handles for output |
| Connection_Name | Connection identifier or N/A |
| Channel_Name | Communication channel name or N/A |
| User_ID | User executing operation or N/A |
| Application_Tag | Application identifier or N/A |
| Timestamp | Report generation timestamp |

### CSV Example

```csv
Queue_Manager,Queue_Name,Queue_Type,Queue_Depth,Handle_Type,Process_ID,IP_Procs,OP_Procs,Open_Input_Count,Open_Output_Count,Connection_Name,Channel_Name,User_ID,Application_Tag,Timestamp
MQQM1,APP1.REQ,LOCAL,0,READ,1234,1,0,1,0,127.0.0.1(1234),APP1.SVRCONN,mqadmin,APP1,2026-02-12 21:47:34
MQQM1,APP1.REQ,LOCAL,0,WRITE,5678,0,1,0,1,127.0.0.1(5678),APP1.SVRCONN,mqadmin,APP1,2026-02-12 21:47:34
MQQM1,APP2.REQ,LOCAL,5,READ/WRITE,9012,1,1,1,1,127.0.0.1(9012),APP1.SVRCONN,mqadmin,N/A,2026-02-12 21:47:34
```

---

## Batch Processing

For processing multiple queue managers efficiently, use batch processing with an input file.

### Batch File Format

Create a text file (e.g., `qm_list.txt`) with one queue manager name per line:

```
MQQM1
PROD_QM
DEV_QM
STAGING_QM
```

### Running Batch Operations

```powershell
# Windows
.\run.ps1 -Config config.toml -BatchFile qm_list.txt

# Linux
./run.sh --config config.toml --batch-file qm_list.txt
```

### Batch Processing Features

- **Sequential Processing:** Queue managers processed one at a time
- **Validation:** Each queue manager verified to exist in configuration before processing
- **Multi-threading:** Internal thread pool processes queues concurrently (controlled by `max_threads`)
- **Consolidated Logging:** All operations logged to single log file
- **Combined CSV Reports:** Option to generate consolidated or separate CSV files
- **Error Handling:** Continues processing on individual failures, reports summary
- **Progress Summary:** Displays overall results and statistics

### Batch Processing Example

```powershell
# Process all queue managers in batch
.\run.ps1 -Config config.toml -BatchFile qm_list.txt

# Output will show:
# [2026-02-12 21:50:00] [INFO] Processing batch file: qm_list.txt
# [2026-02-12 21:50:00] [INFO] Found 4 queue managers to process
# [2026-02-12 21:50:02] [INFO] MQQM1: 5 local queues processed
# [2026-02-12 21:50:05] [INFO] PROD_QM: 12 local queues processed
# [2026-02-12 21:50:08] [INFO] DEV_QM: 3 local queues processed
# [2026-02-12 21:50:11] [INFO] STAGING_QM: 8 local queues processed
# [2026-02-12 21:50:11] [INFO] Batch processing complete. Summary saved.
```

---

## Troubleshooting

### Connection Issues

**Problem:** Cannot connect to queue manager

**Solution:**
1. Verify the queue manager is running and accessible
2. Check host and port values in `config.toml`
3. Verify the channel name is correct and configured for client connections
4. Test network connectivity: `ping <host>`
5. Check firewall rules allow port 1414 (or configured port)
6. Review error log for specific error messages

### Configuration Issues

**Problem:** Queue manager not found in configuration

**Solution:**
1. Verify `[queuemanager.NAME]` section exists in `config.toml`
2. Ensure queue manager name in command matches section exactly (case-sensitive)
3. Validate TOML syntax is correct (proper brackets and formatting)
4. Verify all required parameters are present

### Queue Access Issues

**Problem:** Cannot open queue

**Solution:**
1. Verify queue name is spelled correctly
2. Confirm queue exists on the queue manager
3. Check user has proper permissions to access the queue
4. Verify queue is not restricted to system administration
5. Check queue is not already open in incompatible mode

### Build Issues

**Problem:** Compilation or linking failures

**Solution:**
1. Verify CMake is installed: `cmake --version`
2. Verify GCC/MinGW is installed and accessible
3. Check IBM MQ development headers are installed
4. Verify paths in `CMakeLists.txt` match your MQ installation
5. Try clean rebuild: Use `-Clean` flag in build script
6. Review error messages for specific path issues

### File/Logging Issues

**Problem:** No log output or cannot create log file

**Solution:**
1. Verify `log_file_path` in `config.toml` is correct
2. Ensure log directory exists or can be created
3. Check file system permissions for write access
4. Verify sufficient disk space available
5. Review directory path uses forward slashes or escaped backslashes

---

## Architecture

MQQStatusTool is organized into focused, well-defined components:

### Core Components

**MQLog**
- Handles all logging operations with timestamps
- Implements log file rotation and backup management
- Provides methods for info(), warning(), and error() level logging
- Manages log file size and automatic rotation

**MQConfiguration**
- Parses TOML configuration files
- Provides access to all configuration settings
- Manages multiple queue manager configurations
- Handles global application settings

**MQConnection**
- Manages queue manager connections and lifecycle
- Handles MQ API initialization and cleanup
- Opens and closes queues
- Provides message handles for operations

**MQQueueLister**
- Lists all local queues on a queue manager
- Retrieves detailed queue status using PCF inquiries
- Formats queue information for display
- Identifies reader/writer processes by operation type

**MQCSVWriter**
- Generates CSV reports with queue status data
- Manages file creation and formatting
- Includes queue manager name and timestamp
- Handles multi-process segregation per PID

### Design Patterns

- **Separation of Concerns:** Each class has single responsibility
- **Modular Design:** Components are loosely coupled
- **Resource Management:** RAII pattern for cleanup
- **Error Handling:** Comprehensive error reporting and logging
- **Thread Safety:** Multi-threaded queue manager processing

---

## Platform Support

### Windows

- **Supported Versions:** Windows 10, Windows 11, Windows Server 2016+
- **Build Tool:** PowerShell Core or Windows PowerShell 5.1+
- **Compiler:** MinGW GCC (64-bit)
- **Scripts:** `build.ps1`, `run.ps1`
- **Architecture:** 64-bit executables

### Linux/Unix

- **Supported Distributions:** Ubuntu 18.04+, CentOS 7+, RHEL 7+, Debian 10+
- **Build Tool:** Bash shell, Make, CMake
- **Compiler:** GCC 5.0+ or Clang 3.8+
- **Scripts:** `build.sh`, `run.sh`
- **Architecture:** 64-bit executables

### macOS

- **Supported Versions:** macOS 10.15+
- **Build Tool:** Bash shell, Make, CMake
- **Compiler:** Clang (Xcode Command Line Tools) or GCC
- **Scripts:** `build.sh`, `run.sh` (Bash compatible)
- **Requirements:** Xcode Command Line Tools or GCC installation

---

## Quick Start

### Step 1: Install Prerequisites

**Windows:**
```powershell
# Install CMake
choco install cmake

# Install MinGW GCC
choco install mingw

# Install IBM MQ Client from IBM website
# https://www.ibm.com/products/mq
```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential

# CentOS/RHEL
sudo yum install cmake gcc-c++ make

# Install IBM MQ Client from IBM website
```

### Step 2: Create Required Directories

Create directories as specified in your `config.toml`:

```powershell
# Windows
mkdir logs
mkdir output

# Linux
mkdir -p logs
mkdir -p output
```

### Step 3: Configure Queue Managers

Edit or create `config.toml`:

```toml
[global]
log_file_path = "logs/MQQStatusTool.log"
log_file_size_mb = 10
log_backups = 5
generate_csv = true
csv_file_path = "output/queue_status.csv"
max_threads = 5

[queuemanager.MQQM1]
queue_manager = "MQQM1"
host = "127.0.0.1"
port = 5200
channel = "APP1.SVRCONN"
queue_name = "APP1.REQ"
```


### Step 4: Run Application

```powershell
# Windows - Get all queue status
.\run.ps1 -Config config.toml -Qm MQQM1

# Linux - Get all queue status
./run.sh --config config.toml --qm MQQM1
```

### Step 5: Check Results

1. **Console Output:** Displays queue information immediately
2. **Log File:** `logs/MQQStatusTool.log` contains detailed operation log
3. **CSV Report:** `output/queue_status_MQQM1_*.csv` (if enabled)

---

## Support and Issues

For help with issues or questions:

1. **Check Documentation:** Review this README for relevant sections
2. **Review Logs:** Check `logs/MQQStatusTool.log` for detailed error information
3. **Verify Configuration:** Ensure `config.toml` contains correct settings
4. **Verify Connectivity:** Test queue manager accessibility and port availability
5. **Check Permissions:** Ensure user has proper MQ access permissions
6. **Review Troubleshooting:** See Troubleshooting section for common solutions

---

## License

This software is provided under the Apache License 2.0. See the LICENSE file in the repository root for complete terms and conditions.

### Summary

- **Type:** Apache License 2.0
- **Permissions:** Commercial use, distribution, modification, private use
- **Conditions:** License and copyright notice required
- **Limitations:** No trademark use, no warranty, no liability

### Copyright and Attribution

```
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
```

---

## Contributing

Contributions are welcome. Please ensure:

- Code follows the existing style and conventions
- All changes are documented
- Tests are provided for new functionality
- Documentation is updated accordingly

---

## Disclaimer

This utility is provided as-is without warranty. Users assume all responsibility for proper use and compliance with their organization's policies and IBM MQ licensing requirements.


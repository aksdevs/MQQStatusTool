#!/bin/bash

# MQQStatusTool Run Script for Unix/Linux
# Runs the compiled executable with specified parameters

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
EXE="${BUILD_DIR}/MQQStatusTool"
CONFIG_FILE="${PROJECT_ROOT}/config.toml"
INPUT_FILE="${PROJECT_ROOT}/queue_managers.txt"

function show_help() {
    cat << EOF
MQQStatusTool Run Script
========================

Usage: ./run.sh [OPTIONS]

Options:
  --config <file>     Path to TOML config file (default: config.toml)
  --qm <name>         Queue manager name (must exist in config)
  --queue <name>      Queue name to operate on
  --get               Get queue status and messages (default)
  --put               Put test message on queue
  --input-file <file> Text file with queue manager names for batch processing
  -h, --help          Show this help message

Examples:
  ./run.sh --config config.toml --qm default --queue APP1.REQ --get
  ./run.sh --config config.toml --qm default --queue APP1.REQ --put
  ./run.sh --input-file queue_managers.txt

EOF
}

function print_status() {
    echo -e "${GREEN}OK:${NC} $1"
}

function print_error() {
    echo -e "${RED}ERROR:${NC} $1"
}

function print_info() {
    echo -e "${CYAN}INFO:${NC} $1"
}

function test_executable() {
    if [ ! -f "$EXE" ]; then
        print_error "Executable not found: $EXE"
        echo "Run './build.sh' to build the project first"
        return 1
    fi
    return 0
}

# Parse help argument
if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

# Main
echo -e "${CYAN}MQQStatusTool Runner${NC}"
echo -e "${CYAN}====================${NC}"
echo ""

if ! test_executable; then
    exit 1
fi

print_status "Executable: $EXE"
print_status "Config: $CONFIG_FILE"
echo ""

# Prepare arguments
args=("--config" "$CONFIG_FILE")

# If arguments provided, append them
if [ $# -gt 0 ]; then
    args+=("$@")
fi

print_info "Running: $EXE ${args[@]}"
echo ""

# Run the executable
"$EXE" "${args[@]}"
exitCode=$?

echo ""
if [ $exitCode -eq 0 ]; then
    print_status "Execution completed successfully"
else
    print_error "Execution failed with exit code: $exitCode"
fi

exit $exitCode


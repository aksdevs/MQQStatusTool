#!/bin/bash

# MQQStatusTool Build Script for Unix/Linux
# Uses CMake and GCC/Clang

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
CONFIGURATION="${1:-Release}"
CLEAN_BUILD=false

# IBM MQ paths
MQ_INSTALL_PATH="/opt/mqm"
MQ_TOOLS_PATH="/opt/mqm"

# Check for arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        Release|Debug)
            CONFIGURATION="$1"
            shift
            ;;
        *)
            shift
            ;;
    esac
done

function show_help() {
    cat << EOF
MQQStatusTool Build Script
==========================

Usage: ./build.sh [OPTIONS]

Options:
  Release|Debug       Build configuration (default: Release)
  -c, --clean         Clean build artifacts before building
  -h, --help          Show this help message

Examples:
  ./build.sh                    # Build in Release mode
  ./build.sh Debug              # Build in Debug mode
  ./build.sh --clean            # Clean and rebuild

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

function test_prerequisites() {
    print_info "Checking prerequisites..."

    # Check CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed or not in PATH"
        echo "Install CMake from: https://cmake.org/download/"
        return 1
    fi
    print_status "CMake found: $(cmake --version | head -n1)"

    # Check GCC or Clang
    if command -v g++ &> /dev/null; then
        print_status "GCC found: $(g++ --version | head -n1)"
    elif command -v clang++ &> /dev/null; then
        print_status "Clang found: $(clang++ --version | head -n1)"
    else
        print_error "Neither GCC nor Clang found"
        echo "Install GCC: sudo apt-get install build-essential (Ubuntu/Debian)"
        echo "Or install Clang: sudo apt-get install clang"
        return 1
    fi

    # Check IBM MQ installation
    if [ ! -d "$MQ_TOOLS_PATH" ]; then
        print_error "IBM MQ tools not found at $MQ_TOOLS_PATH"
        echo "Install IBM MQ or update MQ_TOOLS_PATH in this script"
        return 1
    fi
    print_status "IBM MQ tools found"

    return 0
}

function clean_build() {
    print_info "Cleaning build artifacts..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_status "Cleaned"
    fi
}

function build_project() {
    print_info "Building project..."

    # Create build directory
    mkdir -p "$BUILD_DIR"

    cd "$BUILD_DIR"

    # Run CMake
    print_info "Running CMake..."
    cmake -DCMAKE_BUILD_TYPE="$CONFIGURATION" \
          -DIBM_MQ_INSTALL_PATH="$MQ_INSTALL_PATH" \
          -DIBM_MQ_TOOLS_PATH="$MQ_TOOLS_PATH" \
          ..

    if [ $? -ne 0 ]; then
        print_error "CMake failed"
        cd "$PROJECT_ROOT"
        return 1
    fi
    print_status "CMake completed"

    # Build with make
    print_info "Building with make..."
    make

    if [ $? -ne 0 ]; then
        print_error "Build failed"
        cd "$PROJECT_ROOT"
        return 1
    fi
    print_status "Build completed"

    # Check executable
    if [ -f "$BUILD_DIR/MQQStatusTool" ]; then
        print_status "Executable created: $BUILD_DIR/MQQStatusTool"
        cd "$PROJECT_ROOT"
        return 0
    else
        print_error "Executable not found"
        cd "$PROJECT_ROOT"
        return 1
    fi
}

# Main
echo -e "${CYAN}MQQStatusTool Build Script${NC}"
echo -e "${CYAN}===========================${NC}"
echo ""

if ! test_prerequisites; then
    exit 1
fi

if [ "$CLEAN_BUILD" = true ]; then
    clean_build
fi

if ! build_project; then
    exit 1
fi

echo ""
print_status "Build completed successfully!"
echo -e "${CYAN}Executable: $BUILD_DIR/MQQStatusTool${NC}"
echo -e "${CYAN}Run with: ./run.sh${NC}"


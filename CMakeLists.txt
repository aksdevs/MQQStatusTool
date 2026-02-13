cmake_minimum_required(VERSION 3.20)
project(MQQStatusTool C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable RC compiler (Windows resource compiler) to avoid path issues with MinGW
if(MINGW)
    set(CMAKE_RC_COMPILER_INIT "NOTFOUND")
    set(CMAKE_RC_COMPILER NOTFOUND)
endif()

# Threading support
find_package(Threads REQUIRED)

# Platform-specific IBM MQ paths
if(WIN32)
    set(IBM_MQ_INSTALL_PATH "C:/Program Files/IBM/MQ" CACHE PATH "IBM MQ installation path")
    set(IBM_MQ_TOOLS_PATH "C:/Program Files/IBM/MQ/tools" CACHE PATH "IBM MQ tools path")
else()
    set(IBM_MQ_INSTALL_PATH "/opt/mqm" CACHE PATH "IBM MQ installation path")
    set(IBM_MQ_TOOLS_PATH "/opt/mqm" CACHE PATH "IBM MQ tools path")
endif()

# Include directories for MQ C headers (cmqc.h, cmqcfc.h, cmqxc.h)
include_directories(${IBM_MQ_TOOLS_PATH}/c/include)
include_directories(${IBM_MQ_TOOLS_PATH}/cplus/include)

# Add the executable
add_executable(MQQStatusTool src/main.cpp)

# Link threading
target_link_libraries(MQQStatusTool PRIVATE Threads::Threads)

# Platform-specific linking
if(WIN32)
    # The MQ client library (mqic) for client connections
    set(MQ_IMPORT_LIB "${IBM_MQ_TOOLS_PATH}/lib64/mqic.LIB")
    if(EXISTS "${MQ_IMPORT_LIB}")
        message(STATUS "Found MQ client import lib: ${MQ_IMPORT_LIB}")
        target_link_libraries(MQQStatusTool PRIVATE "${MQ_IMPORT_LIB}" ws2_32)
    else()
        message(WARNING "MQ import lib not found at ${MQ_IMPORT_LIB}, trying fallback paths")
        set(MQ_IMPORT_LIB_ALT "${IBM_MQ_INSTALL_PATH}/lib64/mqic.lib")
        if(EXISTS "${MQ_IMPORT_LIB_ALT}")
            target_link_libraries(MQQStatusTool PRIVATE "${MQ_IMPORT_LIB_ALT}" ws2_32)
        else()
            target_link_libraries(MQQStatusTool PRIVATE mqic ws2_32)
        endif()
    endif()
else()
    # Unix/Linux: link against mqic_r (threaded client library)
    link_directories(${IBM_MQ_INSTALL_PATH}/lib64)
    target_link_libraries(MQQStatusTool PRIVATE mqic_r)
endif()

# Print configuration info
message(STATUS "IBM MQ Install Path: ${IBM_MQ_INSTALL_PATH}")
message(STATUS "IBM MQ Tools Path: ${IBM_MQ_TOOLS_PATH}")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")

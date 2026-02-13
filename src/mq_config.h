#ifndef MQ_CONFIG_H
#define MQ_CONFIG_H

/**
 * IBM MQ Connection Configuration
 *
 * Edit this file to modify connection parameters for your MQ environment
 */

// Queue Manager Configuration
#define MQ_QUEUE_MANAGER    "MQQM1"
#define MQ_HOST             "127.0.0.1"
#define MQ_PORT             "5200"
#define MQ_CHANNEL          "APP1.SVRCONN"

// Queue Configuration
#define MQ_QUEUE_NAME       "APP1.REQ"

// Message Retrieval Options
#define MQ_MESSAGE_TIMEOUT  5000      // Timeout in milliseconds
#define MQ_MESSAGE_BUFFER_SIZE 4096   // Maximum message size in bytes
#define MQ_DISPLAY_LIMIT    100       // Characters to display from each message

// Advanced Options (defaults are usually fine)
#define MQ_USE_SSL          0          // Set to 1 to enable SSL/TLS
#define MQ_USE_USERNAME     0          // Set to 1 if authentication required
#define MQ_USERNAME         ""         // Username for authentication
#define MQ_PASSWORD         ""         // Password for authentication

#endif // MQ_CONFIG_H


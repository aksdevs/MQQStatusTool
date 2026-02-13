#ifndef MQ_ADVANCED_FEATURES_H
#define MQ_ADVANCED_FEATURES_H

#include <cmqc.h>
#include <cmqxc.h>
#include <string>
#include <vector>
#include <cstring>

namespace MQAdvanced {

    /**
     * Send a message to a queue
     */
    class MessageSender {
    public:
        static MQLONG sendMessage(MQHCONN hConn, const std::string& queueName,
                                  const std::string& messageData)
        {
            MQHOBJ hQueue = MQHO_UNUSABLE_HOBJ;
            MQLONG CompCode = MQCC_OK;
            MQLONG Reason = MQRC_NONE;

            MQOD queueDesc = {MQOD_DEFAULT};
            strncpy(queueDesc.ObjectName, queueName.c_str(), MQ_Q_NAME_LENGTH);

            MQOPEN(hConn, &queueDesc, MQOO_OUTPUT, &hQueue, &CompCode, &Reason);

            if (CompCode != MQCC_OK) {
                return Reason;
            }

            MQPMO putMsgOpts = {MQPMO_DEFAULT};
            MQMD msgDesc = {MQMD_DEFAULT};

            MQPUT(hConn, hQueue, &msgDesc, &putMsgOpts, (MQLONG)messageData.length(),
                  (MQPTR)messageData.c_str(), &CompCode, &Reason);

            MQLONG putReason = Reason;
            MQCLOSE(hConn, &hQueue, MQCO_NONE, &CompCode, &Reason);

            return putReason;
        }
    };

    /**
     * Browse messages without removing them from the queue
     */
    class MessageBrowser {
    public:
        static void browseMessages(MQHCONN hConn, MQHOBJ hQueue, int maxMessages)
        {
            MQLONG CompCode = MQCC_OK;
            MQLONG Reason = MQRC_NONE;

            MQGMO browseMsgOpts = {MQGMO_DEFAULT};
            browseMsgOpts.Options = MQGMO_BROWSE_FIRST;

            MQMD msgDesc = {MQMD_DEFAULT};
            unsigned char msgBuffer[4096];
            MQLONG dataLength;

            for (int i = 0; i < maxMessages; i++) {
                memset(msgBuffer, 0, sizeof(msgBuffer));
                msgDesc = {MQMD_DEFAULT};

                MQGET(hConn, hQueue, &msgDesc, &browseMsgOpts, sizeof(msgBuffer),
                      msgBuffer, &dataLength, &CompCode, &Reason);

                if (CompCode != MQCC_OK) {
                    break;
                }

                browseMsgOpts.Options = MQGMO_BROWSE_NEXT;

                std::cout << "Message " << (i + 1) << ": "
                          << std::string((char*)msgBuffer, dataLength) << std::endl;
            }
        }
    };

    /**
     * SSL/TLS Connection Configuration
     * CipherSpec is set on the MQCD (channel definition), not the MQSCO.
     * MQSCO holds the key repository path.
     */
    class SSLConfiguration {
    public:
        static void configureSSL(MQCD& clientConn, MQSCO& sslConfig,
                                 const std::string& keyRepository,
                                 const std::string& cipherSpec)
        {
            sslConfig = {MQSCO_DEFAULT};
            strncpy(sslConfig.KeyRepository, keyRepository.c_str(), 256);

            strncpy(clientConn.SSLCipherSpec, cipherSpec.c_str(), MQ_SSL_CIPHER_SPEC_LENGTH);
        }
    };

    /**
     * Connection with Authentication
     * MQCSP uses CSPUserIdPtr/CSPUserIdLength and CSPPasswordPtr/CSPPasswordLength
     */
    class AuthenticationConfig {
    public:
        static void configureAuth(MQCSP& secParams,
                                 const char* userId, MQLONG userIdLen,
                                 const char* password, MQLONG passwordLen)
        {
            secParams = {MQCSP_DEFAULT};
            secParams.AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
            secParams.CSPUserIdPtr = (MQPTR)userId;
            secParams.CSPUserIdLength = userIdLen;
            secParams.CSPPasswordPtr = (MQPTR)password;
            secParams.CSPPasswordLength = passwordLen;
        }
    };

    /**
     * Connection pooling and session management
     */
    class ConnectionPool {
    private:
        std::vector<MQHCONN> connections;
        size_t maxConnections;

    public:
        ConnectionPool(size_t max) : maxConnections(max) {}

        MQHCONN getConnection()
        {
            if (!connections.empty()) {
                MQHCONN conn = connections.back();
                connections.pop_back();
                return conn;
            }
            return MQHC_UNUSABLE_HCONN;
        }

        void returnConnection(MQHCONN conn)
        {
            if (connections.size() < maxConnections) {
                connections.push_back(conn);
            }
        }

        void closeAll()
        {
            MQLONG compCode, reason;
            for (auto conn : connections) {
                MQDISC(&conn, &compCode, &reason);
            }
            connections.clear();
        }
    };

} // namespace MQAdvanced

#endif // MQ_ADVANCED_FEATURES_H

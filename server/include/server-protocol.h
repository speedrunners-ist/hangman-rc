#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

// Below, general error messages related to establishing connections

#define INTERNAL_ERROR "[ERR]: Internal error while processing request."
#define TCP_LISTEN_ERROR "[ERR]: Failed to listen to TCP socket."
#define TCP_ACCEPT_ERROR "[ERR]: Failed to accept TCP connection."
#define TCP_READ_ERROR "[ERR]: Failed to read from TCP socket."
#define FORK_ERROR "[ERR]: Failed to fork process."

// Expected amount of arguments for the SNG command.
#define SNG_ARGS 2

// Expected amount of arguments for the PLG command.
#define PLG_ARGS 4

// Expected amount of arguments for the PWG command.
#define PWG_ARGS 4

// Expected amount of arguments for the QUT command.
#define QUT_ARGS 2

// Expected amount of arguments for the GSB command.
#define GSB_ARGS 1

// Expected amount of arguments for the GHL command.
#define GHL_ARGS 2

// Expected amount of arguments for the STA command.
#define STA_ARGS 2

/**
 * @brief Sets up the server's parameters, according to the program's parameters.
 *
 * @param filePath Path to the file containing the game's words.
 * @param vParam Whether or not the server should operate in a verbose manner.
 */
int setServerParameters(std::string filePath, bool vParam);

/**
 * @brief Closes the UDP socket and exits the program.
 *
 * @param signum The signal's number.
 */
void signalHandlerUDP(int signum);

/**
 * @brief Closes the TCP socket and exits the program.
 *
 * @param signum The signal's number.
 */
void signalHandlerTCP(int signum);

/**
 * @brief Closes the TCP child socket and exits the program.
 *
 * @param signum The signal's number.
 */
void signalHandlerTCPchild(int signum);

/**
 * @brief Centralized UDP communication handler with the client.
 *
 * @param peer Peer information.
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalUDPHandler(peerInfo peer);

/**
 * @brief Centralized TCP communication handler with the client.
 *
 * @param peer Peer information.
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalTCPHandler(peerInfo peer);

/**
 * @brief Handles SNG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleSNG(protocolMessage message);

/**
 * @brief Handles PLG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handlePLG(protocolMessage message);

/**
 * @brief Handles PWG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handlePWG(protocolMessage message);

/**
 * @brief Handles QUT request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleQUT(protocolMessage message);

/**
 * @brief Handles GSB request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleGSB(protocolMessage message);

/**
 * @brief Handles GHL request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleGHL(protocolMessage message);

/**
 * @brief Handles GWD request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleSTA(protocolMessage message);

/**
 * @brief Creates a new UDP socket.
 *
 * @param type The type of socket to be created.
 * @param peer The peer to be connected to.
 * @param handler The signal handler to be used.
 * @return The socket's file descriptor.
 */
int createSocket(__socket_type type, peerInfo peer, sighandler_t handler);

/**
 * @brief Provides specific server-side socket disconnection.
 *
 * @param socket The socket to be disconnected.
 * @return 0 if the socket was successfully disconnected, -1 otherwise.
 */
int disconnect(socketInfo socket);

/**
 * @brief Retrieves a server-side socket, according to its type.
 *
 * @param type The type of socket to be retrieved.
 * @return The socketInfo struct containing the socket's main information.
 */
socketInfo getSocket(__socket_type type);

/**
 * @brief Retrieves the client's address information (UDP).
 *
 * @param type The type of socket to be retrieved.
 * @return The addrinfo struct containing the client's address information.
 */
struct addrinfo *getResUDP();

/**
 * @brief Retrieves the client's address information (TCP).
 *
 * @param type The type of socket to be retrieved.
 * @return The addrinfo struct containing the client's address information.
 */
struct addrinfo *getResTCP();

/**
 * @brief Retrieves the client's file descriptor (UDP).
 *
 * @return The client's file descriptor.
 */
int getSocketFdUDP();

/**
 * @brief Retrieves the client's file descriptor (TCP).
 *
 * @return The client's file descriptor.
 */
int getSocketFdTCP();

/**
 * @brief Check whether the server is operating in a verbose manner.
 *
 * @return True if the server is operating in a verbose manner, false otherwise.
 */
bool checkVerbose();

#endif /* SERVER_PROTOCOL_H */

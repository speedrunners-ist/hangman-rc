#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

// Below, general error messages related to establishing connections

#define INTERNAL_ERROR "[ERR]: Internal error while processing request."
#define TCP_LISTEN_ERROR "[ERR]: Failed to listen to TCP socket."
#define TCP_ACCEPT_ERROR "[ERR]: Failed to accept TCP connection."
#define TCP_READ_ERROR "[ERR]: Failed to read from TCP socket."

// Maximum number of pending TCP connection requests
#define MAX_TCP_CONNECTION_REQUESTS 5

/**
 * @brief Sets up the server's UDP parameters, according to the program's parameters.
 *
 * @param filepath Path to the file containing the game's words.
 * @param vParam Whether or not the server should operate in a verbose manner.
 */
int setServerUDPParameters(std::string filepath, bool vParam);

/**
 * @brief Sets up the server's TCP parameters, according to the program's parameters.
 *
 * @param vParam Whether or not the server should operate in a verbose manner.
 */
int setServerTCPParameters(bool vParam);

/**
 * @brief Sets up the server's UDP socket.
 *
 * @return 0 if the setup was successful, -1 otherwise.
 */
int createSocketUDP(struct peerInfo peer);

/**
 * @brief Sets up the server's TCP socket.
 *
 * @return 0 if the setup was successful, -1 otherwise.
 */
int createSocketTCP(struct peerInfo peer);

/**
 * @brief Disconnects the server from the UDP socket.
 *
 * @return 0 if the disconnection was successful, -1 otherwise.
 */
int disconnectUDP();

/**
 * @brief Disconnects the server from the TCP socket.
 *
 * @return 0 if the disconnection was successful, -1 otherwise.
 */
int disconnectTCP();

/**
 * @brief Centralized UDP communication handler with the client.
 *
 * @param peer Peer information.
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalUDPHandler(struct peerInfo peer);

/**
 * @brief Centralized TCP communication handler with the client.
 *
 * @param peer Peer information.
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalTCPHandler(struct peerInfo peer);

/**
 * @brief Parses a TCP message and sends it to the appropriate handler.
 *
 * @param request TCP message.
 * @return 0 if the message was parsed successfully, -1 otherwise.
 */
int parseTCPMessage(std::string request);

/**
 * @brief Handles SNG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleSNG(struct protocolMessage message);

/**
 * @brief Handles PLG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handlePLG(struct protocolMessage message);

/**
 * @brief Handles PWG request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handlePWG(struct protocolMessage message);

/**
 * @brief Handles QUT request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleQUT(struct protocolMessage message);

/**
 * @brief Handles REV request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleREV(struct protocolMessage message);

/**
 * @brief Handles GSB request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleGSB(struct protocolMessage message);

/**
 * @brief Handles GHL request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleGHL(struct protocolMessage message);

/**
 * @brief Handles GWD request from the client.
 *
 * @param message The message from the client.
 * @return 0 if the request was handled successfully, -1 otherwise.
 */
int handleSTA(struct protocolMessage message);

#endif /* SERVER_PROTOCOL_H */

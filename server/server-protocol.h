#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

#define INTERNAL_ERROR "[ERR]: Internal error while processing request."
#define TCP_LISTEN_ERROR "[ERR]: Failed to listen to TCP socket."
#define TCP_ACCEPT_ERROR "[ERR]: Failed to accept TCP connection."
#define TCP_READ_ERROR "[ERR]: Failed to read from TCP socket."
#define MAX_TCP_CONNECTION_REQUESTS 5

int setServerUDPParameters(std::string filepath, bool vParam);
int setServerTCPParameters(bool vParam);

int createSocketUDP(struct peerInfo peer);
int createSocketTCP(struct peerInfo peer);
int generalUDPHandler(struct peerInfo peer);
int parseTCPMessage(std::string request);
int generalTCPHandler(struct peerInfo peer);

// UDP Server message handlers
int handleSNG(struct protocolMessage message);
int handlePLG(struct protocolMessage message);
int handlePWG(struct protocolMessage message);
int handleQUT(struct protocolMessage message);
int handleREV(struct protocolMessage message);

// TCP Server message handlers
int handleGSB(struct protocolMessage message);
int handleGHL(struct protocolMessage message);
int handleSTA(struct protocolMessage message);

#endif /* SERVER_PROTOCOL_H */

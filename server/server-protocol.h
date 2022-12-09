#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

#define INTERNAL_ERROR "[ERR]: Internal error while processing request."

int setServerParameters(std::string filepath, bool vParam);

void createSocketUDP(std::string addr, std::string port);
int generalUDPHandler(std::string message);

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

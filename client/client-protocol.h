#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

// UDP related functions
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

#endif /* CLIENT_PROTOCOL_H */

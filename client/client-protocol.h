#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

int newSocket(int type, std::string addr, std::string port);

// UDP related functions
int exchangeUDPMessage(int fd, std::string message, struct addrinfo *serverAddr, char *response);
int parseUDPResponse(char *response);

#endif /* CLIENT_PROTOCOL_H */

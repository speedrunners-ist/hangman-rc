#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"
#include "common/common.h"

int newSocket(struct addrinfo *&serverInfo, int type, std::string addr, std::string port);

int exchangeMessageUDP(int fd, std::string message, struct addrinfo *serverAddr, char *response);

int parseUDPResponse(char *response, std::string &message, Play &play);

#endif /* CLIENT_PROTOCOL_H */

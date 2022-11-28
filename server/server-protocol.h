#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."

typedef std::map<std::string, std::function<int(struct protocolMessage request)>> commandHandler;
typedef std::map<std::string, std::function<int(std::string input)>> responseHandler;

void createSocketUDP(std::string addr, std::string port);

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

// UDP client message senders
int sendRSG(std::string input);
int sendRLG(std::string input);
int sendRWG(std::string input);
int sendRQT(std::string input);
int sendRRV(std::string input);

// TCP Client message senders
int sendRSB(std::string input);
int sendRHL(std::string input);
int sendRST(std::string input);

#endif /* SERVER_PROTOCOL_H */

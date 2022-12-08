#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."

// UDP Error Messages - should we really include RSG/RLG/... here? It shouldn't be
// something the player should know about, I think
#define SENDTO_ERROR "[ERR]: Failed to send message to server."
#define RECVFROM_ERROR "[ERR]: Failed to receive message from server."
#define UDP_RESPONSE_ERROR "[ERR]: Response from server does not match the UDP protocol."
#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."
#define RSG_ERROR "[ERR]: Response from server does not match RSG protocol."
#define RLG_ERROR "[ERR]: Response from server does not match RLG protocol."
#define RLG_INVALID_WORD_LEN "[ERR]: Response from server includes invalid word length."
#define RWG_ERROR "[ERR]: Response from server does not match RWG protocol."

typedef std::map<std::string, std::function<int(struct protocolMessage request)>> commandHandler;
typedef std::map<std::string, std::function<int(std::string input)>> responseHandler;

int setServerParameters(std::string filepath, bool vParam);

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

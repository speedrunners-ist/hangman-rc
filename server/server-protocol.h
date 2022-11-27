#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "server-api.h"

#include <stdlib.h>
#include <string.h>

void openUDP(std::string GSport);
void openTCP(std::string GSport);

struct clientRequest {
  std::string code;
  size_t codePos;
  std::string status;
  size_t statusPos;
  std::string body;
};

// UDP utils functions
int generalUDPHandler(std::string message);
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

// TCP utils functions
int generalTCPHandler(std::string message);
int exchangeTCPMessage(std::string message, char *response);
int parseTCPResponse(char *response);

// Server message handlers
int handleSNG(struct clientRequest message);
int handlePLG(struct clientRequest message);
int handlePWG(struct clientRequest message);
int handleGSB(struct clientRequest message);
int handleGHL(struct clientRequest message);
int handleSTA(struct clientRequest message);
int handleQUT(struct clientRequest message);
int handleREV(struct clientRequest message);

// UDP client message handlers
int sendRSG(std::string input);
int sendRLG(std::string input);
int sendRWG(std::string input);
int sendRQT(std::string input);
int sendRRV(std::string input);

// TCP Client message handlers
int sendRSB(std::string input);
int sendRHL(std::string input);
int sendRST(std::string input);

#endif /* SERVER_PROTOCOL_H */
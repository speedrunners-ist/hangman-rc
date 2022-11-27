#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096
#define TCP_READ_SIZE 4096

typedef std::map<std::string, std::function<int(std::string message, std::string input)>>
    messageHandler;
typedef std::map<std::string, std::function<int(std::string message)>> responseHandler;

int newSocket(int type, std::string addr, std::string port);

// UDP utils functions
int generalUDPHandler(std::string message);
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

// TCP utils functions
int generalTCPHandler(std::string message);
int exchangeTCPMessage(std::string message, char *response);
int parseTCPResponse(char *response);

// Player message handlers
// TODO: try to find a better way to handle functions with two arguments
int handleStart(std::string message, std::string input);
int handlePlay(std::string message, std::string input);
int handleGuess(std::string message, std::string input);
int handleScoreboard(std::string message, std::string input);
int handleHint(std::string message, std::string input);
int handleState(std::string message, std::string input);
int handleQuit(std::string message, std::string input);
int handleExit(std::string message, std::string input);
int handleDebug(std::string message, std::string input);

// UDP server message handlers
int handleRSG(std::string response);
int handleRLG(std::string response);
int handleRWG(std::string response);
int handleRQT(std::string response);
int handleRRV(std::string response);

// TCP server message handlers
int handleRSB(std::string response);
int handleRHL(std::string response);
int handleRST(std::string response);

// clang-format off
static messageHandler handlePlayerMessage = {
  {"start", handleStart}, {"sg", handleStart},
  {"play", handlePlay}, {"pl", handlePlay},
  {"guess", handleGuess}, {"gw", handleGuess},
  {"scoreboard", handleScoreboard}, {"sb", handleScoreboard},
  {"hint", handleHint}, {"h", handleHint},
  {"state", handleState}, {"st", handleState},
  {"quit", handleQuit}, {"exit", handleExit}, {"rev", handleDebug}
};
// clang-format on

#endif /* CLIENT_PROTOCOL_H */

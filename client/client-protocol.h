#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096
#define TCP_READ_SIZE 4096

int newSocket(int type, std::string addr, std::string port);

// UDP related functions
int generalUDPHandler(std::string message);
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

// TCP related functions
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

static messageHandler handlePlayerMessage = {{"start", handleStart},
                                             {"sg", handleStart},
                                             {"play", handlePlay},
                                             {"pl", handlePlay},
                                             {"guess", handleGuess},
                                             {"gw", handleGuess},
                                             {"scoreboard", handleScoreboard},
                                             {"sb", handleScoreboard},
                                             {"hint", handleHint},
                                             {"h", handleHint},
                                             {"state", handleState},
                                             {"st", handleState},
                                             {"quit", handleQuit},
                                             {"exit", handleExit},
                                             {"rev", handleDebug}};

#endif /* CLIENT_PROTOCOL_H */

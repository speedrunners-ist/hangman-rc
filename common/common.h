#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_GSIP "tejo.tecnico.ulisboa.pt"
#define DEFAULT_GSPORT "58011"

#define MAX_USER_INPUT 1024

struct protocolMessage {
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

// Below, function prototypes

/*
 * @brief: Amount of wrong guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
unsigned int initialAvailableMistakes(unsigned int wordLength);

#endif /* COMMON_H */

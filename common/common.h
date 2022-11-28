#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <numeric>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define DEFAULT_GSIP "tejo.tecnico.ulisboa.pt"
#define DEFAULT_GSPORT "58001"

#define MAX_USER_INPUT 1024

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096
#define TCP_READ_SIZE 4096

#define SOCKET_ERROR "[ERR]: Failed to create socket. Exiting."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info. Exiting."

struct protocolMessage {
  std::string code;
  size_t codePos;
  std::string status;
  size_t statusPos;
  std::string body;
};

int newSocket(int type, std::string addr, std::string port, struct addrinfo **serverInfo);

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
std::string buildMessage(std::vector<std::string> args);

#endif /* COMMON_H */

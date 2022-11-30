#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
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
#define DEFAULT_GSPORT "58011"

// sizes below are arbitrary, in bytes
#define UDP_RECV_SIZE 1024
#define TCP_CHUNK_SIZE 1024
#define MAX_USER_INPUT 1024

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define SOCKET_TIMEOUT 5

#define SOCKET_ERROR "[ERR]: Failed to create socket. Exiting."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info. Exiting."

struct protocolMessage {
  std::string code;
  size_t codePos;
  std::string status;
  size_t statusPos;
  std::string body;
};

struct fileInfo {
  std::string fileName;
  int fileSize;
  char delimiter;
};

int newSocket(int type, std::string addr, std::string port, struct addrinfo **serverInfo);
int disconnectUDP();
int disconnectTCP();

int turnOnSocketTimer(int socketFd);
int turnOffSocketTimer(int socketFd);

// UDP utils functions
int generalUDPHandler(std::string message);
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

// TCP utils functions
int generalTCPHandler(std::string message);
int exchangeTCPMessage(std::string message, char *response);
int sendTCPMessage(std::string message);
int receiveTCPMessage(std::string &message, int args);
int receiveTCPFile(std::string &message, std::string dir);
int parseTCPResponse(char *response);
int parseFileArgs(struct fileInfo &info);

// Below, function prototypes

/*
 * @brief: Amount of wrong guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
unsigned int initialAvailableMistakes(unsigned int wordLength);
std::string buildSplitString(std::vector<std::string> args);
int displayFileRank(std::string fileName);
int displayFile(std::string fileName);

#endif /* COMMON_H */

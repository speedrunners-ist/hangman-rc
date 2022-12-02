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
#define TCP_CHUNK_SIZE 1024
#define MAX_USER_INPUT 1024

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define SOCKET_TIMEOUT 5

#define SOCKET_ERROR "[ERR]: Failed to create socket. Exiting."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info. Exiting."

#define TCP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close TCP socket."
#define TCP_SEND_MESSAGE_ERROR "[ERR]: Failed to send message via TCP."
#define TCP_RECV_MESSAGE_ERROR "[ERR]: Failed to receive message via TCP."
#define TCP_FILE_ARGS_ERROR "[ERR]: Failed to receive file arguments."

#define UDP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close UDP socket."

#define INVALID_FILE_ARGS "[ERR]: Arguments for file transfer are invalid."
#define FILE_OPEN_ERROR "[ERR]: Failed to open file."
#define FILE_RECV_SUCCESS "[INFO]: File received successfully."

#define SB_DIR "scoreboard/"
#define H_DIR "hints/"
#define ST_DIR "state/"

#define EXIT_PROGRAM "[INFO]: Exiting program."

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

struct peerInfo {
  std::string addr;
  std::string port;
};

struct messageInfo {
  std::string input;
  struct peerInfo peer;
};

int newSocket(int type, std::string addr, std::string port, struct addrinfo **serverInfo);
int disconnectUDP();
int disconnectTCP();

int turnOnSocketTimer(int socketFd);
int turnOffSocketTimer(int socketFd);

// UDP utils functions
int generalUDPHandler(std::string message, size_t maxExpectedBytes);
int exchangeUDPMessage(std::string message, char *response, size_t maxExpectedBytes);
int parseUDPResponse(char *response);

// TCP utils functions
int generalTCPHandler(std::string message, struct peerInfo peer);
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
int displayFile(std::string fileName, std::string dir);

#endif /* COMMON_H */

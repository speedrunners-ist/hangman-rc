#ifndef COMMON_H
#define COMMON_H

#include <algorithm>
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

typedef std::map<char, bool> Alphabet;

class GameState {
protected:
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
  int trials = 0; // TODO: STANDARDIZE THIS BETWEEN SERVER AND CLIENT
  int spotsLeft;
  char lastGuess;
  bool active = false;
  std::string lastWordGuess;
  Alphabet guessedLetters;
  std::string word;

public:
  GameState();
  GameState(int length, int mistakes);
  void incrementTrials();
  int getTrials();
  bool isActive();
  void setInactive();
  int getAvailableMistakes();
  char getLastGuess();
  std::string getLastWordGuess();
  int getWordLength();
  std::string getWord();
  void setLastGuess(char guess);
  void setLastWordGuess(std::string guess);
  void setWord(std::string newWord);
  void incorrectGuess();
  int correctGuess(std::string positions, int n);
  void correctFinalGuess();
  void correctFinalWordGuess();
  bool isLetterGuessed(char letter);
  void setSpotsLeft(int spots);
  int getSpotsLeft();
};

#define DEFAULT_GSIP "tejo.tecnico.ulisboa.pt"
#define DEFAULT_GSPORT "58001"

// sizes below are arbitrary, in bytes
#define TCP_CHUNK_SIZE 1024
#define MAX_USER_INPUT 1024
#define UDP_RECV_SIZE 1024

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define SOCKET_TIMEOUT 5

#define SOCKET_ERROR "[ERR]: Failed to create socket."
#define SOCKET_TIMER_SET_ERROR "[ERR]: Failed to set socket timeout."
#define SOCKET_TIMER_RESET_ERROR "[ERR]: Failed to reset socket timeout."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info."
#define BIND_ERROR "[ERR]: Failed to bind socket."
#define SENDTO_ERROR "[ERR]: Failed to send message."
#define RECVFROM_ERROR "[ERR]: Failed to receive message."

#define TCP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close TCP socket."
#define TCP_SEND_MESSAGE_ERROR "[ERR]: Failed to send message via TCP."
#define TCP_RECV_MESSAGE_ERROR "[ERR]: Failed to receive message via TCP."
#define TCP_FILE_ARGS_ERROR "[ERR]: Failed to receive file arguments."

#define UDP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close UDP socket."
#define UDP_PARSE_ERROR "[ERR]: Found error while parsing the message."
#define UDP_RESPONSE_ERROR "[ERR]: Message does not match the UDP protocol."

#define INVALID_FILE_ARGS "[ERR]: Arguments for file transfer are invalid."
#define FILE_OPEN_ERROR "[ERR]: Failed to open file."
#define FILE_RECV_SUCCESS "[INFO]: File received successfully."

#define SB_DIR "scoreboard/"
#define H_DIR "hints/"
#define ST_DIR "state/"

#define EXIT_PROGRAM "[INFO]: Exiting program."

#define UDP_HANGMAN_ERROR "[ERR]: Message body does not match any expected protocols."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_POSITIONS_ERROR "[ERR]: Server response includes invalid positions."
#define ALREADY_FILLED_ERROR "[ERR]: Position already filled."
#define DIFF_POSITIONS_ERROR "[ERR]: Expected a different amount of positions than the ones given."
#define EXPECTED_POSITIONS(n, m) "[ERR]: Expected " << n << " positions, but got " << m << "."
#define CORRECT_GUESS(word) "[INFO]: Correct guess. Word is now: " << word

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

int newSocket(int type, std::string addr, std::string port, struct addrinfo *hints,
              struct addrinfo **serverInfo);
int disconnectUDP(struct addrinfo *res, int fd);
int disconnectTCP();

int turnOnSocketTimer(int socketFd);
int turnOffSocketTimer(int socketFd);

// UDP utils functions
int exchangeUDPMessages(std::string message, char *response, size_t maxBytes, struct addrinfo *res, int fd);
int sendUDPMessage(std::string message, struct addrinfo *res, int fd);
int parseUDPMessage(std::string message, struct protocolMessage &response);

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
int initialAvailableMistakes(int wordLength);
std::string buildSplitString(std::vector<std::string> args);
int displayFile(std::string fileName, std::string dir);
int validateArgsAmount(std::string input, int n);
int validatePlayerID(std::string id);
bool forceExit(GameState play, std::string command);
void continueReading(char *buffer);

#endif /* COMMON_H */

#ifndef COMMON_H
#define COMMON_H

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <numeric>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

typedef std::map<char, bool> Alphabet;
typedef std::map<std::string, bool> WordList;
typedef std::map<std::string, std::function<int(struct messageInfo info)>> commandHandler;
typedef std::map<std::string, std::function<int(struct protocolMessage response)>> responseHandler;

class GameState {
protected:
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
  int trials = 0;
  int spotsLeft;
  char lastGuess;
  bool active = false;
  std::string playerID;
  std::string lastWordGuess;
  Alphabet guessedLetters;
  WordList guessedWords;
  std::string word;
  std::string hint;

public:
  GameState();
  GameState(int length, int mistakes, std::string plid);
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
  std::string getPlayerID();
  void setPlayerID(std::string id);
  bool isLetterGuessed(char letter);
  void setSpotsLeft(int spots);
  int getSpotsLeft();
  void setHint(std::string newHint);
  std::string getHint();
  void addGuessedLetter(char letter);
  void addGuessedWord(std::string word);
  void setMistakesLeft(int mistakes);
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

#define SIGNAL_MESSAGE(signum) "[INFO]: Received signal " << signum << ". Exiting program."
#define PROMPT "> "

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
#define TCP_FILE_SEND_ERROR "[ERR]: Failed to send file."
#define TCP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close TCP socket."
#define TCP_PARSE_ERROR "[ERR]: Found error while parsing the message."
#define TCP_RESPONSE_ERROR "[ERR]: Message does not match the TCP protocol."

#define UDP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close UDP socket."
#define UDP_PARSE_ERROR "[ERR]: Found error while parsing the message."
#define UDP_RESPONSE_ERROR "[ERR]: Message does not match the UDP protocol."

#define INVALID_FILE_ARGS "[ERR]: Arguments for file transfer are invalid."
#define FILE_OPEN_ERROR "[ERR]: Failed to open file."
#define FILE_RECV_SUCCESS "[INFO]: File received successfully."

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
  std::string first;
  size_t firstPos;
  std::string second;
  size_t secondPos;
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
int disconnectTCP(struct addrinfo *res, int fd);

int turnOnSocketTimer(int socketFd);
int turnOffSocketTimer(int socketFd);

// UDP utils functions
int exchangeUDPMessages(std::string message, char *response, size_t maxBytes, struct addrinfo *res, int fd);
int sendUDPMessage(std::string message, struct addrinfo *res, int fd);
int parseUDPMessage(std::string message, struct protocolMessage &response);

// TCP utils functions
int generalTCPHandler(std::string message, struct peerInfo peer);
int sendTCPMessage(std::string message, int fd);
int sendTCPFile(std::string message, int fd, std::string filePath);
int receiveTCPMessage(std::string &message, int args, int fd);
int receiveTCPFile(struct fileInfo &info, std::string dir, int fd);

// Below, function prototypes

/*
 * @brief: Amount of wrong guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
int initialAvailableMistakes(int wordLength);
std::string buildSplitString(std::vector<std::string> args);
std::string buildSplitStringNewline(std::vector<std::string> args);
int readFile(std::vector<std::string> &lines, std::string filePath);
int displayFile(std::string filePath, std::string dir);
int validateArgsAmount(std::string input, int n);
bool validPlayerID(std::string id);
bool forceExit(GameState play, std::string command);
void continueReading(char *buffer);
void signalHandler(int signum);

#endif /* COMMON_H */

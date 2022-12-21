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

/**
 * @brief Struct that represents a message received.
 *
 * Used to store the message received in a neatly formatted way, such that we can
 * easily retrieve both the command and the status (and, if needed, the whole message's
 * body) from a given message.
 *
 * @param command The message's command (e.g. "RSG OK").
 * @param request The message's general request (e.g. "RSG").
 * @param status The message's status (e.g. "OK").
 * @param args The message's arguments (e.g. "5 10").
 * @param body The body of the message.
 */
typedef struct {
  std::string command;
  std::string request;
  std::string status;
  std::string args;
  std::string body;
} protocolMessage;

/**
 * @brief Struct that represents a file, containing associated relevant information.
 *
 * @param fileName The name of the file.
 * @param fileSize The size of the file.
 * @param delimiter The delimiter used in the protocol's message after displaying the file's arguments.
 */
typedef struct {
  std::string fileName;
  size_t fileSize;
  char delimiter;
} fileInfo;

/**
 * @brief Struct that represents a peer, containing associated relevant information.
 *
 * @param addr The IP address of the peer.
 * @param port The port of the peer.
 */
typedef struct {
  std::string addr;
  std::string port;
} peerInfo;

/**
 * @brief Struct utilized in order to store the core information needed in order
 * to send a message to a peer: the peer's information and the message itself.
 *
 * @param input The message to be sent.
 * @param peer The peer to which the message will be sent.
 */
typedef struct {
  std::string input;
  peerInfo peer;
} messageInfo;

/**
 * @brief Struct that represents a socket, containing associated relevant information.
 *
 * @param res The socket's address information.
 * @param hints The socket's hints.
 * @param act The action to be taken when a signal is received.
 * @param isConnected Whether the socket is connected or not.
 * @param created Whether the socket has been created or not.
 * @param type The socket's type.
 * @param fd The socket's file descriptor.
 */
typedef struct {
  struct addrinfo *res;
  struct addrinfo hints;
  struct sigaction act;
  bool isConnected;
  bool created;
  __socket_type type;
  int fd;
} socketInfo;

// Redirection handler for each specific request.
typedef std::map<std::string, std::function<int(protocolMessage response)>> responseHandler;

/**
 * @brief Class that represents a game state.
 *
 * Used to store the state of a game, and to provide methods to manipulate it.
 * Stores useful information, ranging from the word's length to the
 * number of mistakes left and the letters/words previously guessed.
 */
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
  std::map<char, bool> guessedLetters;
  std::map<std::string, bool> guessedWords;
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
  bool isWordGuessed(std::string word);
  void setMistakesLeft(int mistakes);
};

// Default value for the server's IP address, if none is provided.
#define DEFAULT_GSIP "localhost"
// Default value for the server's port, if none is provided.
#define DEFAULT_GSPORT "58045"

// Maximum number of characters in a single user message (client-side, via stdin).
#define MAX_USER_INPUT 1024
// Maximum number of Bytes to be sent in a single UDP message.
#define UDP_RECV_SIZE 1024
// Maximum number of Bytes to be sent in a single TCP message (chunk).
#define TCP_CHUNK_SIZE 1024

// When exiting the program (i.e using "exit" vs "quit"), the client's UDP handler will return this value.
#define EXIT_HANGMAN 1

// Default socket timeout value (in seconds).
#define SOCKET_TIMEOUT 5

// Maximum number of pending TCP connection requests
#define MAX_TCP_CONNECTION_REQUESTS 5

// Below, a series of error messages to be displayed to the user via stderr.
// Errors range from socket creation and handling, TCP/UDP connections to file handling.

#define SIGNAL(signum) "[INFO]: Interrupt signal (" << signum << ") received."

#define PORT_ERROR "[ERR]: Invalid port number."
#define IP_ERROR "[ERR]: Invalid IP address."

#define SOCKET_ERROR "[ERR]: Failed to create socket."
#define SOCKET_CLOSE_ERROR "[ERR]: Failed to close socket."
#define SOCKET_REUSE_ERROR "[ERR]: Failed to set socket re-usage flag."
#define SOCKET_TIMER_SET_ERROR "[ERR]: Failed to set socket timeout."
#define SOCKET_TIMER_RESET_ERROR "[ERR]: Failed to reset socket timeout."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info."
#define BIND_ERROR "[ERR]: Failed to bind socket."
#define SENDTO_ERROR "[ERR]: Failed to send message."
#define RECVFROM_ERROR "[ERR]: Failed to receive message."
#define SIGACTION_ERROR "[ERR]: Failed to set action."
#define CONNECTION_ERROR "[ERR]: Failed to connect to peer."

#define ERR "ERR\n"

#define TCP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close TCP socket."
#define TCP_SEND_MESSAGE_ERROR "[ERR]: Failed to send message via TCP."
#define TCP_RECV_MESSAGE_ERROR "[ERR]: Failed to receive message via TCP."
#define TCP_FILE_ARGS_ERROR "[ERR]: Failed to receive file arguments."
#define TCP_FILE_SEND_ERROR "[ERR]: Failed to send file."
#define TCP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close TCP socket."
#define TCP_PARSE_ERROR "[ERR]: Found error while parsing the message."
#define TCP_RESPONSE_ERROR "[ERR]: Message does not match the TCP protocol."

#define UDP_SOCKET_CLOSE_ERROR "[ERR]: Failed to close UDP socket."
#define PARSE_ERROR "[ERR]: Found error while parsing the message."
#define UDP_RESPONSE_ERROR "[ERR]: Message does not match the UDP protocol."
#define UDP_HANGMAN_ERROR "[ERR]: Server message does not match any expected protocols."

#define INVALID_FILE_ARGS "[ERR]: Arguments for file transfer are invalid."
#define FILE_OPEN_ERROR "[ERR]: Failed to open file."
#define FILE_RECV_SUCCESS "[INFO]: File received successfully."
#define FILE_DOES_NOT_EXIST "[ERR]: File does not exist."

#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_POSITIONS_ERROR "[ERR]: Server response includes invalid positions."
#define ALREADY_FILLED_ERROR "[ERR]: Position already filled."
#define DIFF_POSITIONS_ERROR "[ERR]: Expected a different amount of positions than the ones given."
#define EXPECTED_POSITIONS(n, m) "[ERR]: Expected " << n << " positions, but got " << m << "."

#define DEFAULT_GSIP_STR "[INFO] Could not find IP address, using default IP: " << DEFAULT_GSIP
#define DEFAULT_GSPORT_STR "[INFO] Could not find Gsport number, using default port: " << DEFAULT_GSPORT

#define EXIT_PROGRAM "[INFO]: Exiting program."
#define CORRECT_GUESS(word) "[INFO]: Correct guess. Word is now: " << word

#define DELETE_FILE_ERROR(filePath) "[ERR]: Failed to delete file " << filePath << "."
#define DELETE_FILE_SUCCESS(filePath) "[INFO]: File " << filePath << " deleted successfully."

// Amount of expected arguments in a TCP server response.
#define TCP_DEFAULT_ARGS 2

// Amount of expected file information arguments in a TCP server response.
#define TCP_FILE_ARGS 2

/**
 * @brief Checks if the given port number is valid.
 *
 * @param port
 * @return int
 */
int checkPortNumber(std::string port);

/**
 * @brief Creates a new socket.
 *
 * @param type Type of socket to be created - SOCK_DGRAM or SOCK_STREAM.
 * @param peer PeerInfo struct containing the peer's IP address and port.
 * @param hints Hints struct containing the socket's family and type.
 * @param serverInfo Pointer to a struct addrinfo that will be filled with the server's address info.
 * @return The socket's file descriptor.
 */
int newSocket(__socket_type type, peerInfo peer, struct addrinfo *hints, struct addrinfo **serverInfo);

/**
 * @brief Handles the creation of a socket (and everything surrounding it).
 *
 * @param type Type of socket to be created - SOCK_DGRAM or SOCK_STREAM.
 * @param peer PeerInfo struct containing the peer's IP address and port.
 * @param handler Signal handler to be set associated with the socket.
 * @param isClient Boolean indicating whether the socket is for a client or a server.
 * @return A socketInfo struct containing the socket's main information.
 */
socketInfo handleSocketCreation(__socket_type type, peerInfo peer, sighandler_t handler, bool isClient);

/**
 * @brief Handle graceful disconnection of a socket.
 *
 * @param res Pointer to a struct addrinfo containing the socket's address info.
 * @param fd Socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int disconnectSocket(struct addrinfo *res, int fd);

/**
 * @brief Handle initialization of a socket's timeout.
 *
 * @param socketFd Socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int turnOnSocketTimer(int socketFd);

/**
 * @brief Handle reset of a socket's timeout.
 *
 * @param socketFd Socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int turnOffSocketTimer(int socketFd);

/**
 * @brief Handle parsing of a message sent from a socket.
 *
 * @param message Message to be parsed.
 * @param response Struct to store the parsed message.
 * @param fullMessage Whether the message being handled is a full message or not.
 * @return 0 on success, -1 on error.
 */
int parseMessage(std::string message, protocolMessage &response, bool fullMessage = true);

/**
 * @brief Handle sending of a UDP message.
 *
 * @param message Message to be sent.
 * @param res The socket's address info.
 * @param fd The socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int sendUDPMessage(std::string message, struct addrinfo *res, int fd);

/**
 * @brief Handle receiving of a UDP message.
 *
 * @param response Buffer to store the received message.
 * @param maxBytes Maximum amount of bytes to be received.
 * @param res The socket's address info.
 * @param fd The socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int receiveUDPMessage(char *response, size_t maxBytes, struct addrinfo *res, int fd);

/**
 * @brief Handle receiving of a parsed UDP message (with possible send of a response).
 *
 * @param message The previously parsed message.
 * @param handler The function to be called to handle the message.
 * @param fd The socket's file descriptor.
 * @param res The socket's address info.
 * @return 0 on success, -1 on error.
 */
int messageUDPHandler(protocolMessage &message, responseHandler handler, int fd = -1,
                      struct addrinfo *res = NULL);

/**
 * @brief Handle sending of a TCP message.
 *
 * @param message Message to be sent.
 * @param res The socket's address info.
 * @param fd The socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int sendTCPMessage(std::string message, struct addrinfo *res, int fd);

/**
 * @brief Handle sending of a file via TCP.
 *
 * @param info File information (name and size).
 * @param res The socket's address info.
 * @param fd The socket's file descriptor.
 * @param filePath Path to the file to be sent.
 * @return 0 on success, -1 on error.
 */
int sendTCPFile(std::string info, struct addrinfo *res, int fd, std::string filePath);

/**
 * @brief Handle receiving of a TCP message.
 *
 * @param message Buffer to store the received message.
 * @param args Amount of expected arguments in the message.
 * @param fd The socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int receiveTCPMessage(std::string &message, int args, int fd);

/**
 * @brief Handle receiving of a file via TCP.
 *
 * @param info File information (name and size).
 * @param dir Directory to store the received file.
 * @param fd The socket's file descriptor.
 * @return 0 on success, -1 on error.
 */
int receiveTCPFile(fileInfo &info, std::string dir, int fd);

/**
 * @brief Handle receiving of a parsed TCP message (with possible send of a response).
 *
 * @param message The previously parsed message.
 * @param handler The function to be called to handle the message.
 * @param fd The socket's file descriptor.
 * @param res The socket's address info.
 * @return 0 on success, -1 on error.
 */
int messageTCPHandler(protocolMessage &message, responseHandler handler, int fd = -1,
                      struct addrinfo *res = NULL);

/**
 * @brief Parses the server-sent file information for the following file transfer.
 *
 * @param info The struct that will hold the file information.
 * @param fd The socket's file descriptor.
 * @return 0 if the file information was successfully parsed, -1 otherwise.
 */
int parseFileArgs(fileInfo &info, int fd);

/**
 * @brief Amount of wrong guesses a player can make before failing.
 *
 * @param wordLength: Length of the word to guess.
 * @return Number of available mistakes to be made.
 */
int initialAvailableMistakes(int wordLength);

/**
 * @brief Builds a string split by spaces.
 *
 * @param args: Vector of to-be-joined strings.
 * @return String with all the strings joined by spaces.
 */
std::string buildSplitString(std::vector<std::string> args);

/**
 * @brief Builds a string split by spaces, with a newline at the end.
 *
 * @param args: Vector of to-be-joined strings.
 * @return String with all the strings joined by spaces, with a newline at the end.
 */
std::string buildSplitStringNewline(std::vector<std::string> args);

/**
 * @brief Reads a given file's content line by line.
 *
 * @param lines: Vector of strings (lines) to be filled with the file's content.
 * @param filePath: Path to the file to be read.
 * @return 0 on success, -2 if the file does not exist, -1 on error.
 */
int readFile(std::vector<std::string> &lines, std::string filePath);

/**
 * @brief Reads a given file's content line by line.
 *
 * @param filePath: Path to the file to be read.
 * @return 0 on success, -1 on error.
 */
int displayFile(std::string filePath);

/**
 * @brief Checks if a string has a given amount of arguments - strings split by spaces.
 *
 * @param input: String to be checked.
 * @param n: Expected amount of arguments.
 * @return True if the string has the expected amount of arguments, false otherwise.
 */
bool validArgsAmount(std::string input, int n);

/**
 * @brief Checks if a string has a given amount of arguments - strings split by spaces.
 * If the string has the expected amount of arguments, the arguments are stored in a vector.
 *
 * @param body: String to be checked.
 * @param args: Vector of strings to be filled with the arguments.
 * @param expectedArgs: Expected amount of arguments.
 * @return True if the string has the expected amount of arguments, false otherwise.
 */
bool gatherResponseArguments(std::string body, std::vector<int> &args, int expectedArgs);

/**
 * @brief Forces the exit of the program.
 *
 * @param state: To-be-checked state of the program.
 * @param command: Command to be checked.
 * @return True if the program should exit (i.e game isn't active and the user pressed exit), false
 * otherwise.
 */
bool forceExit(GameState state, std::string command);

/**
 * @brief Prints a prompt to the user and clears a given buffer.
 *
 * @param buffer: Buffer to be cleared.
 */
void continueReading(char *buffer);

/**
 * @brief Converts a given string to lowercase.
 *
 * @param str: String to be converted.
 */
void toLower(std::string &str);

/**
 * @brief Checks if a given string is a valid PLID (exactly 6 digits).
 *
 * @param plid: String to be checked.
 * @return True if the string has the correct PLID format, false otherwise.
 */
bool validPlayerID(std::string plid);

/**
 * @brief Checks if a given string has a number format.
 *
 * @param trial: String to be checked.
 * @return True if the string has a number format, false otherwise.
 */
bool isNumber(std::string trial);

/**
 * @brief Checks if a given string is a valid word (all alphanumeric characters).
 *
 * @param word: String to be checked.
 * @return True if the string is a valid word, false otherwise.
 */
bool hasWordFormat(std::string word);

#endif /* COMMON_H */

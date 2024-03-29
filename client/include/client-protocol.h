#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

// Redirection handler for each specific user command.
typedef std::map<std::string, std::function<int(messageInfo info)>> commandHandler;

// Expected amount of arguments for the "start" command.
#define START_ARGS 2

// Expected amount of arguments for the "play" command.
#define PLAY_ARGS 2

// Expected amount of arguments for the "guess" command.
#define GUESS_ARGS 2

// Expected amount of arguments for the quit and "exit" commands.
#define QUIT_ARGS 1

// Expected amount of arguments for the "scoreboard" command.
#define SCOREBOARD_ARGS 1

// Expected amount of arguments for the "hint" command.
#define HINT_ARGS 1

// Expected amount of arguments for the "state" command.
#define STATE_ARGS 1

// NOTE: the Byte amounts below include spaces and a newline character.

// Maximum amount of Bytes that can be sent by the server in a RSG message.
#define RSG_BYTES 3 + 1 + 3 + 1 + 2 + 1 + 1 + 1

// Expected amount of arguments (not including command and status) for the RSG message.
#define RSG_ARGS 2

// Maximum amount of Bytes that can be sent by the server in a RLG message.
#define RLG_BYTES 3 + 1 + 3 + 1 + 2 + 2 + 2 * 30 + 1

// Expected amount of arguments (not including command and status) for the RLG message.
#define RLG_ARGS 2

// Maximum amount of Bytes that can be sent by the server in a RWG message.
#define RWG_BYTES 3 + 1 + 3 + 1 + 2 + 1

// Expected amount of arguments (not including command and status) for the RWG message.
#define RWG_ARGS 1

// Maximum amount of Bytes that can be sent by the server in a RQT message.
#define RQT_BYTES 3 + 1 + 3 + 1

// Below, a series of error messages to be displayed to the user via stderr.
// Errors range from general incorrect input usage to command-specific errors.

#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                                  \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."
#define NO_PLAYER_ERROR "[ERR]: There's no playerID currently set. Please start a game."
#define NO_GAME_ERROR "[ERR]: There's no game currently ongoing. Please start a game."
#define UNEXPECTED_MESSAGE "[ERR]: Unexpected message received from server."
#define UNEXPECTED_COMMAND "[ERR]: The command received is not supported by this client."
#define INVALID_PLAYER_ID "[ERR]: Invalid player ID. Please try again."

#define RSG_OK(mistakes, word)                                                                               \
  "New game started (max " + std::to_string(mistakes) + " mistakes allowed). Word to guess: " + word
#define RSG_NOK                                                                                              \
  "Failed to start a new game - a game is currently ongoing. Use quit to leave the current game."
#define RSG_ERROR "[ERR]: Response from server does not match RSG protocol."
#define RSG_ERR "An error occurred - perhaps the submitted player ID is invalid? Try again."
#define RLG_ERROR "[ERR]: Response from server does not match RLG protocol."
#define RLG_INVALID_WORD_LEN "[ERR]: Response from server includes invalid word length."
#define RLG_WIN(word) ("WELL DONE! You guessed: " + word)
#define RLG_DUP "You have already guessed this letter."
#define RLG_NOK(mistakes) "Wrong guess. " + std::to_string(mistakes) + " errors left."
#define RLG_OVR "GAME OVER! You do not have any more errors left."
#define RLG_INV "An invalid trial parameter was sent. Try again."
#define RLG_ERR "An error occurred - is there a game currently ongoing? Is the player ID valid? Try again."

#define RWG_ERROR "[ERR]: Response from server does not match RWG protocol."
#define RWG_WIN(word) "WELL DONE! You guessed: " + word
#define RWG_DUP "You have already guessed this word."
#define RWG_NOK(mistakes) "Wrong guess. " + std::to_string(mistakes) + " errors left."
#define RWG_OVR "GAME OVER! You do not have any more errors left."
#define RWG_INV "An invalid trial parameter was sent. Try again."
#define RWG_ERR "An error occurred. Is there a game currently ongoing? Is the player ID valid? Try again."

#define RQT_OK "Game was successfully quit."
#define RQT_ERR "Failed to quit game."

#define RRV_OK(word) "[REV]: Word is " + word

#define RSB_SUCCESS(path, bytes) "[INFO]: Scoreboard stored at " << path << ", " << bytes << " bytes."
#define RSB_FAIL "[INFO]: The server hasn't held any games yet."
#define RHL_SUCCESS(path, bytes) "[INFO]: Hint stored at " << path << ", " << bytes << " bytes."
#define RHL_FAIL "[INFO]: The server could not send any hints at the moment."
#define RST_SUCCESS(path, bytes) "[INFO]: State stored at " << path << ", " << bytes << " bytes."
#define RST_ACT "[INFO]: Displaying information about the current game."
#define RST_FIN "[INFO]: Displaying information about the last finished game."
#define RST_NOK "[INFO]: The server could not find any games (neither active nor finished)."
#define RST_ERR "[ERR]: The server has encountered an error while processing your request."

#define DISPLAY_ERR "[ERR]: An error has occurred. Please try again later."

// Directory where the scoreboard file will be stored.
#define SB_DIR "client/assets/scoreboard/"

// Directory where the hint files will be stored.
#define H_DIR "client/assets/hints/"

// Directory where the state files will be stored.
#define ST_DIR "client/assets/state/"

// File name for the scoreboard file
#define SB_PATH(name) SB_DIR + name

// File name for the state file
#define ST_PATH(name) ST_DIR + name

/**
 * @brief Centralized UDP communication handler with the server.
 *
 * @param message The message to be sent to the server.
 * @param maxBytes The maximum amount of bytes to be received from the server.
 *
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalUDPHandler(std::string message, size_t maxBytes);

/**
 * @brief Centralized TCP communication handler with the server.
 *
 * @param message The message to be sent to the server.
 * @param peer The peerInfo struct containing the server's IP and port.
 *
 * @return 0 if the communication was successful, -1 otherwise.
 */
int generalTCPHandler(std::string message, peerInfo peer);

/**
 * @brief Handles RSG responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRSG(protocolMessage response);

/**
 * @brief Handles RLG responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRLG(protocolMessage response);

/**
 * @brief Handles RWG responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRWG(protocolMessage response);

/**
 * @brief Handles RQT responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRQT(protocolMessage response);

/**
 * @brief Handles RSB responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRSB(protocolMessage response);

/**
 * @brief Handles RHL responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRHL(protocolMessage response);

/**
 * @brief Handles RST responses from the server.
 *
 * @param response The response from the server.
 * @return 0 if the response was handled successfully, -1 otherwise.
 */
int handleRST(protocolMessage response);

/**
 * @brief Sends SNG messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendSNG(messageInfo info);

/**
 * @brief Sends PLG messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendPLG(messageInfo info);

/**
 * @brief Sends PWG messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendPWG(messageInfo info);

/**
 * @brief Sends QUT messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendQUT(messageInfo info);

/**
 * @brief Sends GSB messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendGSB(messageInfo info);

/**
 * @brief Sends GHL messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendGHL(messageInfo info);

/**
 * @brief Sends STA messages to the server.
 *
 * @param info The messageInfo struct containing the message's parameters.
 * @return 0 if the message was successfully sent and its response handled, -1 otherwise.
 */
int sendSTA(messageInfo info);

// UDP/TCP socket creation and retrieval functions

/**
 * @brief General signal handler (SIGINT, SIGTERM, etc.) for client-side signals.
 *
 * @param signum The signal number.
 */
void signalHandler(int signum);

/**
 * @brief Creates a new socket.
 *
 * @param type The type of socket to be created.
 * @param peer The peer to be connected to.
 * @param handler The signal handler to be used.
 * @return The socket's file descriptor.
 */
int createSocket(__socket_type type, peerInfo peer, sighandler_t handler);

/**
 * @brief Provides specific client-side socket disconnection.
 *
 * @param socket The socket to be disconnected.
 * @return 0 if the socket was successfully disconnected, -1 otherwise.
 */
int disconnect(socketInfo socket);

/**
 * @brief Retrieves a client-side socket, according to its type.
 *
 * @param type The type of socket to be retrieved.
 * @return The socketInfo struct containing the socket's main information.
 */
socketInfo getSocket(__socket_type type);

/**
 * @brief Retrieves the server information (UDP), according to its type.
 *
 * @return The addrinfo struct containing the server's main information (UDP).
 */
struct addrinfo *getServerInfoUDP();

/**
 * @brief Retrieves the server information (TCP), according to its type.
 *
 * @return The addrinfo struct containing the server's main information (TCP).
 */
struct addrinfo *getServerInfoTCP();

/**
 * @brief Retrieves the client's UDP socket file descriptor.
 *
 * @return The client's UDP socket file descriptor.
 */
int getSocketFdUDP();

/**
 * @brief Retrieves the client's TCP socket file descriptor.
 *
 * @return The client's TCP socket file descriptor.
 */
int getSocketFdTCP();

/**
 * @brief Retrieves the expected server-sent message (considering a prior request).
 *
 * @return The expected server-sent message.
 */
std::string getExpectedMessage();

/**
 * @brief Sets a new expected server-sent message (considering a prior request).
 *
 * @param message The new expected server-sent message.
 */
void setExpectedMessage(std::string message);

#endif /* CLIENT_PROTOCOL_H */

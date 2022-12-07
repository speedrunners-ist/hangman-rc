#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

// TODO: If equal to server move to common.h
typedef std::map<std::string, std::function<int(struct messageInfo info)>> commandHandler;
typedef std::map<std::string, std::function<int(struct protocolMessage response)>> responseHandler;

// Expected amount of arguments for each protocol (client-side)
#define START_ARGS 2
#define PLAY_ARGS 2
#define GUESS_ARGS 2
#define QUIT_ARGS 1
#define REVEAL_ARGS 1
#define SCOREBOARD_ARGS 1
#define HINT_ARGS 1
#define STATE_ARGS 1

// Maximum amount of bytes that can be sent by the server for each UDP command
// Macros defined as the sum of each element's max byte amount
// Account for spaces and for the newline in the end
#define RSG_BYTES 3 + 1 + 3 + 1 + 2 + 1 + 1 + 1
#define RLG_BYTES 3 + 1 + 3 + 1  + 1 + 2 + 2 * 30 + 1
#define RWG_BYTES 3 + 1 + 3 + 1 + 1 + 1
#define RQT_BYTES 3 + 1 + 3 + 1
#define RRV_BYTES 3 + 1 + 30 + 1

#define TCP_DEFAULT_ARGS 2
#define TCP_FILE_ARGS 2

#define SENDTO_ERROR "[ERR]: Failed to send message to server."
#define RECVFROM_ERROR "[ERR]: Failed to receive message from server."
#define UDP_RESPONSE_ERROR "[ERR]: Response from server does not match the UDP protocol."
#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."
#define TCP_SERVER_ERROR "[ERR]: Failed to connect to server via TCP."

#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define MKDIR_ERROR(dir) "[ERR]: Failed to create directory " + dir + ". Exiting."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                        \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."
#define UNEXPECTED_COMMAND_ERROR(commands)                                                         \
  "[ERR]: Invalid input. Expected one of the following commands: " + commands

// UDP - client-side specific messages
#define RSG_ERROR "[ERR]: Response from server does not match RSG protocol."
#define RLG_ERROR "[ERR]: Response from server does not match RLG protocol."
#define RLG_INVALID_WORD_LEN "[ERR]: Response from server includes invalid word length."
#define RWG_ERROR "[ERR]: Response from server does not match RWG protocol."
#define RSG_OK(mistakes, word)                                                                     \
  ("New game started (max " + std::to_string(mistakes) +                                           \
   " mistakes allowed). Word to guess: " + word)
#define RSG_NOK "Failed to start a new game. Try again later."
#define RLG_WIN(word) ("WELL DONE! You guessed: " + word)
#define RLG_DUP "You have already guessed this letter."
#define RLG_NOK(mistakes) ("Wrong guess. " + std::to_string(mistakes) + " errors left.")
#define RLG_OVR "GAME OVER! You do not have any more errors left."
#define RLG_INV "An invalid trial parameter was sent. Try again."
#define RLG_ERR "RLG ERR"
#define RWG_WIN(word) ("WELL DONE! You guessed: " + word)
#define RWG_NOK(mistakes) ("Wrong guess. " + std::to_string(mistakes) + " errors left.")
#define RWG_OVR "GAME OVER! You do not have any more errors left."
#define RWG_INV "An invalid trial parameter was sent. Try again."
#define RWG_ERR "RWG ERR"
#define RQT_OK "Game was successfully quit."
#define RQT_ERR "Failed to quit game. Try again later."

// TCP - client-side specific messages
#define SB_HEADER "[RANK]: SCORE | PLID | WORD | CORRECT GUESSES | TOTAL GUESSES"
#define SB_FAIL "[INFO]: The server hasn't held any games yet."
#define H_SUCCESS(filename, bytes) "[HINT]: " << filename << ", " << bytes << " bytes."
#define H_FAIL "[INFO]: The server could not send any hints at the moment."
#define ST_ACT "[INFO]: Displaying information about the current game."
#define ST_FIN "[INFO]: Displaying information about the last finished game."
#define ST_NOK "[INFO]: The server could not find any games (neither active nor finished)."
#define ST_ERR "[ERR]: The server has encountered an error while processing your request."

int createSocketUDP(struct peerInfo peer);
int createSocketTCP(struct peerInfo peer);

// UDP Server message servers
int handleRSG(struct protocolMessage response);
int handleRLG(struct protocolMessage response);
int handleRWG(struct protocolMessage response);
int handleRQT(struct protocolMessage response);
int handleRRV(struct protocolMessage response);

// TCP Server message servers
int handleRSB(struct protocolMessage response);
int handleRHL(struct protocolMessage response);
int handleRST(struct protocolMessage response);

// UDP Client message handlers
int sendSNG(struct messageInfo info);
int sendPLG(struct messageInfo info);
int sendPWG(struct messageInfo info);
int sendQUT(struct messageInfo info);
int sendREV(struct messageInfo info);

// TCP Client message handlers
int sendGSB(struct messageInfo info);
int sendGHL(struct messageInfo info);
int sendSTA(struct messageInfo info);

#endif /* CLIENT_PROTOCOL_H */

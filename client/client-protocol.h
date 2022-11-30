#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

// Expected amount of arguments for each protocol (client-side)
#define START_ARGS 2
#define PLAY_ARGS 2
#define GUESS_ARGS 2
#define QUIT_ARGS 1
#define REVEAL_ARGS 1
#define SCOREBOARD_ARGS 1
#define HINT_ARGS 1
#define STATE_ARGS 1

#define TCP_DEFAULT_ARGS 2
#define TCP_FILE_ARGS 2

// UDP Error Messages - should we really include RSG/RLG/... here? It shouldn't be
// something the player should know about, I think
#define SENDTO_ERROR "[ERR]: Failed to send message to server."
#define RECVFROM_ERROR "[ERR]: Failed to receive message from server."
#define UDP_RESPONSE_ERROR "[ERR]: Response from server does not match the UDP protocol."
#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."
#define RSG_ERROR "[ERR]: Response from server does not match RSG protocol."
#define RLG_ERROR "[ERR]: Response from server does not match RLG protocol."
#define RLG_INVALID_WORD_LEN "[ERR]: Response from server includes invalid word length."
#define RWG_ERROR "[ERR]: Response from server does not match RWG protocol."

// Messages shown to the user
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

// TODO: If equal to server move to common.h
typedef std::map<std::string, std::function<int(std::string input)>> commandHandler;
typedef std::map<std::string, std::function<int(struct protocolMessage response)>> responseHandler;

void createSocketUDP(std::string addr, std::string port);
void createSocketTCP(std::string addr, std::string port);

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
int sendSNG(std::string input);
int sendPLG(std::string input);
int sendPWG(std::string input);
int sendQUT(std::string input);
int sendREV(std::string input);

// TCP Client message handlers
int sendGSB(std::string input);
int sendGHL(std::string input);
int sendSTA(std::string input);

#endif /* CLIENT_PROTOCOL_H */

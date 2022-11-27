#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-game-state.h"

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096
#define TCP_READ_SIZE 4096

// Expected amount of arguments for each protocol (client-side)
#define SNG_ARGS 2
#define PLG_ARGS 4
#define PWG_ARGS 4
#define QUT_ARGS 2
#define REV_ARGS 2
#define GSB_ARGS 1
#define GHL_ARGS 2
#define STA_ARGS 2

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

struct serverResponse {
  std::string code;
  size_t codePos;
  std::string status;
  size_t statusPos;
  std::string body;
};
typedef std::map<std::string, std::function<int(std::string input)>> messageHandler;
typedef std::map<std::string, std::function<int(struct serverResponse response)>> responseHandler;
int newSocket(int type, std::string addr, std::string port);

// UDP utils functions
int generalUDPHandler(std::string message);
int exchangeUDPMessage(std::string message, char *response);
int parseUDPResponse(char *response);

// TCP utils functions
int generalTCPHandler(std::string message);
int exchangeTCPMessage(std::string message, char *response);
int parseTCPResponse(char *response);

// Player message handlers
int handleSNG(std::string input);
int handlePLG(std::string input);
int handlePWG(std::string input);
int handleGSB(std::string input);
int handleGHL(std::string input);
int handleSTA(std::string input);
int handleQUT(std::string input);
int handleREV(std::string input);

// UDP server message handlers
int handleRSG(struct serverResponse response);
int handleRLG(struct serverResponse response);
int handleRWG(struct serverResponse response);
int handleRQT(struct serverResponse response);
int handleRRV(struct serverResponse response);

// TCP server message handlers
int handleRSB(struct serverResponse response);
int handleRHL(struct serverResponse response);
int handleRST(struct serverResponse response);

#endif /* CLIENT_PROTOCOL_H */

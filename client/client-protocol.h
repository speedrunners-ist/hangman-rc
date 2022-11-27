#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "client-api.h"

#define EXIT_HANGMAN 1
#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096
#define TCP_READ_SIZE 4096

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

struct serverResponse {
  std::string code;
  size_t codePos;
  std::string status;
  size_t statusPos;
  std::string body;
};
typedef std::map<std::string, std::function<int(std::string message, std::string input)>>
    messageHandler;
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
// TODO: try to find a better way to handle functions with two arguments
int handleStart(std::string message, std::string input);
int handlePlay(std::string message, std::string input);
int handleGuess(std::string message, std::string input);
int handleScoreboard(std::string message, std::string input);
int handleHint(std::string message, std::string input);
int handleState(std::string message, std::string input);
int handleQuit(std::string message, std::string input);
int handleExit(std::string message, std::string input);
int handleDebug(std::string message, std::string input);

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

// clang-format off
static messageHandler handlePlayerMessage = {
  {"start", handleStart}, {"sg", handleStart},
  {"play", handlePlay}, {"pl", handlePlay},
  {"guess", handleGuess}, {"gw", handleGuess},
  {"scoreboard", handleScoreboard}, {"sb", handleScoreboard},
  {"hint", handleHint}, {"h", handleHint},
  {"state", handleState}, {"st", handleState},
  {"quit", handleQuit}, {"exit", handleExit}, {"rev", handleDebug}
};
// clang-format on

#endif /* CLIENT_PROTOCOL_H */

#ifndef SERVER_API_H
#define SERVER_API_H

#include "common/common.h"

class ServerGameState : public GameState {
public:
  ServerGameState();
  ServerGameState(int length, int mistakes);
  bool isLetterGuessed(char letter);
  void setSpotsLeft(int spots);
  int getSpotsLeft();
};

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./GS file-path [-p GSport] -v"
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                        \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."
#define UNEXPECTED_COMMAND_ERROR(commands)                                                         \
  "[ERR]: Invalid input. Expected one of the following commands: " + commands

// RSG return codes
#define CREATE_GAME_ERROR 1
#define CREATE_GAME_SUCCESS 2

// RLG return codes
#define SUCCESS_GUESS 1
#define SUCCESS_FINAL_GUESS 2
#define DUPLICATE_GUESS 3
#define WRONG_GUESS 4
#define WRONG_FINAL_GUESS 5
#define TRIAL_MISMATCH 6
#define SYNTAX_ERROR 7

// RQT return codes
#define CLOSE_GAME_ERROR 1
#define CLOSE_GAME_SUCCESS 2

ServerGameState createGame(int length, int mistakes);
int getAvailableMistakes(ServerGameState play);
std::string getWord(ServerGameState play);
int playCorrectGuess(ServerGameState play, std::string positions, int n);
void playIncorrectGuess(ServerGameState play);
void playCorrectFinalGuess(ServerGameState play);
void playCorrectFinalWordGuess(ServerGameState play);
void setLastGuess(ServerGameState play, char guess);
void setLastWordGuess(ServerGameState play, std::string guess);
int getWordLength(ServerGameState play);
void setPlayerID(std::string id);
std::string getPlayerID();
void incrementTrials();
int getTrials();

int setPath(std::string filepath);
int createGameSession(std::string plid, std::string &arguments);
int isOngoingGame(std::string plid);

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments);
int getOccurrences(std::string word, char letter, std::string &positions);
int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments);
int closeGameSession(std::string plid);

#endif /* SERVER_API_H */

#ifndef SERVER_API_H
#define SERVER_API_H

#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <functional>

typedef std::map<char, bool> Alphabet;

class GameState {
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
  int trials = 1;
  int spotsLeft;
  char lastGuess;
  bool active = false;
  std::string lastWordGuess;
  Alphabet guessedLetters;
  std::string word;

public:
  GameState();
  GameState(int length, int mistakes);
  bool isActive();
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
  void incrementTrials();
  int getTrials();
  bool isLetterGuessed(char letter);
  void setSpotsLeft(int spots);
  int getSpotsLeft();
};

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define MKDIR_ERROR "[ERR]: Failed to create hints directory. Exiting."
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

GameState createGame(int length, int mistakes);
int getAvailableMistakes(GameState play);
std::string getWord(GameState play);
int playCorrectGuess(GameState play, std::string positions, int n);
void playIncorrectGuess(GameState play);
void playCorrectFinalGuess(GameState play);
void playCorrectFinalWordGuess(GameState play);
void setLastGuess(GameState play, char guess);
void setLastWordGuess(GameState play, std::string guess);
int getWordLength(GameState play);
void setPlayerID(std::string id);
std::string getPlayerID();
void incrementTrials();
int getTrials();

int validateArgsAmount(std::string input, int n);
int validatePlayerID(std::string id);
void exitGracefully(std::string errorMessage);
bool forceExit(std::string command);
void continueReading(char *buffer);

void setPath(std::string filepath);
int createGameSession(std::string plid, std::string &arguments);
int isOngoingGame(std::string plid);

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments);
int getOccurances(std::string word, char letter, std::string &positions);
int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments);
int closeGameSession(std::string plid);

#endif /* SERVER_API_H */
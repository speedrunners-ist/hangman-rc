#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common/common.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

typedef std::map<char, bool> Alphabet;

class GameState {
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
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
  void setInactive();
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
};

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define MKDIR_ERROR(dir) "[ERR]: Failed to create directory " + dir + ". Exiting."
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                        \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."
#define UNEXPECTED_COMMAND_ERROR(commands)                                                         \
  "[ERR]: Invalid input. Expected one of the following commands: " + commands

void createGame(int length, int mistakes);
int getAvailableMistakes();
std::string getWord();
int playCorrectGuess(std::string positions, int n);
void playIncorrectGuess();
void playCorrectFinalGuess();
void playCorrectFinalWordGuess();
void setLastGuess(char guess);
void setLastWordGuess(std::string guess);
int getWordLength();
void setPlayerID(std::string id);
std::string getPlayerID();
void incrementTrials();
void resetGame();
int getTrials();

int validateArgsAmount(std::string input, int n);
int validatePlayerID(std::string id);
void exitGracefully(std::string errorMessage);
bool forceExit(std::string command);
void continueReading(char *buffer);

#endif /* CLIENT_API_H */

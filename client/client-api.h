#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common/common.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

typedef std::map<char, bool> Alphabet;
typedef std::map<int, char> Word;

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define MKDIR_ERROR "[ERR]: Failed to create hints directory. Exiting."
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                        \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."

void createGame(int length, int mistakes);
int getAvailableMistakes();
std::string getWord();
int playCorrectGuess(std::string positions, int n);
void playIncorrectGuess();
void playCorrectFinalGuess();
void setLastGuess(char guess);
int getWordLength();
void setPlayerID(std::string id);
std::string getPlayerID();
void incrementTrials();
int getTrials();

int validateArgsAmount(std::string input, int n);
int validatePlayerID(std::string id);
void exitGracefully(std::string errorMessage);
void continueReading(char *buffer);
std::string buildPlayerMessage(std::vector<std::string> args);

#endif /* CLIENT_API_H */

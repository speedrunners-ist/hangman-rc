#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common/common.h"
#include <numeric>

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
bool forceExitClient(std::string command);

#endif /* CLIENT_API_H */

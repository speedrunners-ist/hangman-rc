#include "client-api.h"

/*** ClientAPI methods implementation ***/

GameState play;
std::string playerID;

// Game state functions, useful for the client's protocol implementations
void createGame(int length, int mistakes) { play = GameState(length, mistakes); }
int getAvailableMistakes() { return play.getAvailableMistakes(); }
std::string getWord() { return play.getWord(); }
int playCorrectGuess(std::string positions, int n) {
  int ret = play.correctGuess(positions, n);
  if (ret == 0) {
    play.incrementTrials();
  }
  return ret;
}
void playIncorrectGuess() {
  incrementTrials();
  play.incorrectGuess();
}
void playCorrectFinalGuess() { play.correctFinalGuess(); }

void playCorrectFinalWordGuess() { play.correctFinalWordGuess(); }

void setLastGuess(char guess) { play.setLastGuess(guess); }
void setLastWordGuess(std::string guess) { play.setLastWordGuess(guess); }
int getWordLength() { return play.getWordLength(); }
void setPlayerID(std::string id) { playerID = id; }
std::string getPlayerID() { return playerID; }
void resetGame() { play.setInactive(); }
void incrementTrials() { play.incrementTrials(); }
int getTrials() { return play.getTrials(); }

bool forceExitClient(std::string command) { return forceExit(play, command); }

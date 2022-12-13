#include "client-api.h"

/*** ClientAPI methods implementation ***/

GameState state;
std::string plid;

void createGame(int length, int mistakes, std::string playerID) {
  state = GameState(length, mistakes, playerID);
}

int playCorrectGuess(std::string positions, int n) {
  int ret = state.correctGuess(positions, n);
  if (ret == 0) {
    state.incrementTrials();
  }
  return ret;
}

void playIncorrectGuess() {
  incrementTrials();
  state.incorrectGuess();
}

void playCorrectFinalGuess() { state.correctFinalGuess(); }

void playCorrectFinalWordGuess() { state.correctFinalWordGuess(); }

int getAvailableMistakes() { return state.getAvailableMistakes(); }

std::string getWord() { return state.getWord(); }

void setLastGuess(char guess) { state.setLastGuess(guess); }

void setLastWordGuess(std::string guess) { state.setLastWordGuess(guess); }

int getWordLength() { return state.getWordLength(); }

void setPlayerID(std::string id) { plid = id; }

std::string getPlayerID() { return plid; }

void resetGame() { state.setInactive(); }

void incrementTrials() { state.incrementTrials(); }

int getTrials() { return state.getTrials(); }

bool forceExitClient(std::string command) { return forceExit(state, command); }

std::vector<std::string> getKeys(commandHandler map) {
  std::vector<std::string> keys;
  for (auto const &pair : map) {
    keys.push_back(pair.first);
  }
  return keys;
}

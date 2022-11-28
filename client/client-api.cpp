#include "client-api.h"

static GameState play = GameState(1, 1);

// Game state functions
void createGame(int length, int mistakes) { play = GameState(length, mistakes); }
int getAvailableMistakes() { return play.getAvailableMistakes(); }
std::string getWord() { return play.getWord(); }
int playCorrectGuess(std::string positions, int n) { return play.correctGuess(positions, n); }
void playIncorrectGuess() { play.incorrectGuess(); }
void playCorrectFinalGuess() { play.correctFinalGuess(); }
void setLastGuess(char guess) { play.setLastGuess(guess); }
int getWordLength() { return play.getWordLength(); }

// Util functions
int validateArgsAmount(std::string input, int n) {
  const long int argCount = std::count(input.begin(), input.end(), ' ');
  // argCount will find every space in the string - ideally, one space less than the args amount
  if (argCount != n - 1 || input.back() != '\n') {
    std::cerr << DIFF_ARGS_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int validatePlayerID(std::string playerID) {
  if (playerID.length() != 6) {
    std::cerr << INVALID_PLID_LEN_ERROR << std::endl;
    return -1;
  }

  for (size_t i = 0; i < playerID.length(); i++) {
    if (!isdigit(playerID[i])) {
      std::cerr << INVALID_PLID_CHAR_ERROR << std::endl;
      return -1;
    }
  }
  return 0;
}

void exitGracefully(std::string errorMessage) {
  std::cerr << errorMessage << std::endl;

  // TODO: close socket in case of error
  // close(fd);
  // freeaddrinfo(serverInfo);
  // exit(EXIT_SUCCESS);
}

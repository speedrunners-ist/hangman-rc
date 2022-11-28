#include "client-api.h"

static GameState play = GameState(1, 1);
static std::string playerID;
static int trials = 0;

// Game state functions
void createGame(int length, int mistakes) { play = GameState(length, mistakes); }
int getAvailableMistakes() { return play.getAvailableMistakes(); }
std::string getWord() { return play.getWord(); }
int playCorrectGuess(std::string positions, int n) {
  int ret = play.correctGuess(positions, n);
  if (ret == 0) {
    incrementTrials();
  }
  return ret;
}
void playIncorrectGuess() {
  incrementTrials();
  play.incorrectGuess();
}
void playCorrectFinalGuess() {
  incrementTrials();
  play.correctFinalGuess();
}
void setLastGuess(char guess) { play.setLastGuess(guess); }
int getWordLength() { return play.getWordLength(); }
void setPlayerID(std::string id) { playerID = id; }
std::string getPlayerID() { return playerID; }
void incrementTrials() { trials++; }
int getTrials() { return trials; }

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

int validatePlayerID(std::string id) {
  if (id.length() != 6) {
    std::cerr << INVALID_PLID_LEN_ERROR << std::endl;
    return -1;
  }

  for (size_t i = 0; i < id.length(); i++) {
    if (!isdigit(id[i])) {
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

std::string buildPlayerMessage(std::vector<std::string> args) {
  // clang-format off
  return std::accumulate(
    ++args.begin(), args.end(), std::string(args[0]),
    [](std::string a, std::string b) { 
      return a + " " + b;
    }
  ).append("\n");
  // clang-format on
}

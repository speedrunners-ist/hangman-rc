#include "client-api.h"

// GameState methods implementation
GameState::GameState() { this->active = false; }
GameState::GameState(int length, int mistakes) {
  this->wordLength = length;
  this->mistakesLeft = mistakes;
  this->active = true;
  // word is a string with length equal to wordLength, filled with underscores
  this->word = std::string((size_t)length, '_');
  for (char c = 'a'; c <= 'z'; c++) {
    guessedLetters[c] = false;
  }
}

bool GameState::isActive() { return active; }

int GameState::getAvailableMistakes() { return mistakesLeft; }

char GameState::getLastGuess() { return lastGuess; }

std::string GameState::getLastWordGuess() { return lastWordGuess; }

int GameState::getWordLength() { return wordLength; }

std::string GameState::getWord() { return word; }

void GameState::setLastGuess(char guess) { lastGuess = guess; }

void GameState::setLastWordGuess(std::string guess) { lastWordGuess = guess; }

void GameState::setWord(std::string newWord) { this->word = newWord; }

void GameState::incorrectGuess() {
  char guess = getLastGuess();
  // TODO: do we have to check if we're setting to true something that's already true?
  guessedLetters[guess] = true;
  guessesMade++;
  mistakesLeft--;
}

int GameState::correctGuess(std::string positions, int n) {
  char guess = getLastGuess();
  std::string initialWord = word; // TODO: check if this is a copy or a reference
  int readPositions = 0;
  size_t pos;
  do {
    pos = positions.find_first_of(" \n");
    std::string posStr = positions.substr(0, pos);
    const size_t posNum = (size_t)std::stoi(posStr);
    if (posNum < 1 || posNum > wordLength) {
      std::cerr << "[ERR]: Server response includes invalid positions." << std::endl;
      setWord(initialWord);
      return -1;
    } else if (word[posNum - 1] != '_') {
      std::cout << "Current word: " << word << std::endl;
      std::cout << "Position " << posNum << " is already filled." << std::endl;
      std::cerr << "[ERR]: Server response includes an already filled position." << std::endl;
      setWord(initialWord);
      return -1;
    }
    word[posNum - 1] = guess;
    positions = positions.substr(pos + 1);
    readPositions++;
  } while (pos != std::string::npos);

  if (n != readPositions) {
    // the answer didn't include as many positions as expected
    std::cerr << "[ERR]: Expected a different amount of positions than the ones given."
              << std::endl;
    std::cerr << "[ERR]: Expected " << n << " positions, but got " << readPositions << "."
              << std::endl;
    setWord(initialWord);
    return -1;
  }
  std::cout << "You guessed correctly! Word is now: " << getWord() << std::endl;
  guessesMade++;
  guessedLetters[guess] = true;
  return 0;
}

void GameState::correctFinalGuess() {
  char guess = getLastGuess();
  guessedLetters[guess] = true;
  // replace all underscores with the guess
  std::replace(word.begin(), word.end(), '_', guess);
  active = false;
}

void GameState::correctFinalWordGuess() {
  word = lastWordGuess;
  active = false;
}

static GameState play;
static std::string playerID;
static int trials = 0;

// Game state functions, useful for the client's protocol implementations
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

void playCorrectFinalWordGuess() {
  incrementTrials();
  play.correctFinalWordGuess();
}

void setLastGuess(char guess) { play.setLastGuess(guess); }
void setLastWordGuess(std::string guess) { play.setLastWordGuess(guess); }
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

bool forceExit(std::string command) { return command == "exit" && !play.isActive(); }

void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}

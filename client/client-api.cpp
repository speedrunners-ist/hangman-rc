#include "client-api.h"

// GameState methods implementation
GameState::GameState(int length, int mistakes) {
  this->wordLength = length;
  this->mistakesLeft = mistakes;
  for (int i = 1; i <= length; i++) {
    word[i] = '_';
  }
  for (char c = 'a'; c <= 'z'; c++) {
    guessedLetters[c] = false;
  }
}

int GameState::getAvailableMistakes() { return mistakesLeft; }

char GameState::getLastGuess() { return lastGuess; }

int GameState::getWordLength() { return wordLength; }

std::string GameState::getWord() {
  std::string wordStr;
  for (int i = 1; i <= wordLength; i++) {
    wordStr += word[i];
  }
  return wordStr;
}

void GameState::setLastGuess(char guess) { lastGuess = guess; }

void GameState::setWord(Word newWord) { this->word = newWord; }

void GameState::incorrectGuess() {
  char guess = getLastGuess();
  // TODO: do we have to check if we're setting to true something that's already true?
  guessedLetters[guess] = true;
  guessesMade++;
  mistakesLeft--;
}

int GameState::correctGuess(std::string positions, int n) {

  char guess = getLastGuess();
  // for every int in positions, set the corresponding (-1) char in word to guess
  // done with std::string.find
  Word initialWord = Word(word);
  int readPositions = 0;
  std::cout << "positions: " << positions << std::endl;
  size_t pos;
  do {
    pos = positions.find_first_of(" \n");
    std::string posStr = positions.substr(0, pos);
    int posInt = std::stoi(posStr);
    if (posInt < 1 || posInt > wordLength) {
      std::cerr << "[ERR]: Server response includes invalid positions." << std::endl;
      setWord(initialWord);
      return -1;
    } else if (word[posInt] != '_') {
      std::cerr << "[ERR]: Server response includes an already filled position." << std::endl;
      setWord(initialWord);
      return -1;
    }
    word[posInt] = guess;
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
  std::cout << "Correct guess! Word is now: " << getWord() << std::endl;
  guessesMade++;
  guessedLetters[guess] = true;
  return 0;
}

void GameState::correctFinalGuess() {
  char guess = getLastGuess();
  guessedLetters[guess] = true;
  for (int i = 0; i < wordLength; i++) {
    if (word[i] == '_') {
      word[i] = guess;
    }
  }
  guessesMade++;
}

static GameState play = GameState(1, 1);
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

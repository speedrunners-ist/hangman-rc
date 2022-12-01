#include "server-api.h"

static std::string filepath;
static std::vector<std::string> lines;
static int totalLines;
static std::map<std::string, GameState> GameSessisons;

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

bool GameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void GameState::setSpotsLeft(int spots) { this->spotsLeft = spots; }

int GameState::getSpotsLeft() { return spotsLeft; }

void GameState::incorrectGuess() {
  // TODO: do we have to check if we're setting to true something that's already true?
  guessesMade++;
  mistakesLeft--;
  incrementTrials();
}

int GameState::correctGuess(std::string positions, int n) {

  guessesMade++;
  // guessedLetters[guess] = true;
  spotsLeft -= n;
  incrementTrials();

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

void GameState::incrementTrials() { trials++; }
int GameState::getTrials() { return trials; }

// Game state functions, useful for the client's protocol implementations
GameState createGame(int length, int mistakes) { return GameState(length, mistakes); }
int getAvailableMistakes(GameState play) { return play.getAvailableMistakes(); }
std::string getWord(GameState play) { return play.getWord(); }

int playCorrectGuess(GameState play, std::string positions, int n) {
  int ret = play.correctGuess(positions, n);
  if (ret == 0) {
    play.incrementTrials();
  }
  return ret;
}
void playIncorrectGuess(GameState play) {
  play.incrementTrials();
  play.incorrectGuess();
}
void playCorrectFinalGuess(GameState play) {
  play.incrementTrials();
  play.correctFinalGuess();
}

void playCorrectFinalWordGuess(GameState play) {
  play.incrementTrials();
  play.correctFinalWordGuess();
}

void setLastGuess(GameState play, char guess) { play.setLastGuess(guess); }
void setLastWordGuess(GameState play, std::string guess) { play.setLastWordGuess(guess); }
int getWordLength(GameState play) { return play.getWordLength(); }

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

bool forceExit(GameState play, std::string command) {
  return command == "exit" && !play.isActive();
}

void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}

int getNumberMistakes(int wordLength) {
  if (wordLength <= 6) {
    return 7;
  }
  if (wordLength <= 10) {
    return 8;
  }
  return 9;
}

int createGameSession(std::string plid, std::string &arguments) {

  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) != 0) {
    return CREATE_GAME_ERROR;
  }

  int randomLineNumber = rand() % totalLines;
  std::string randomLine = lines.at(randomLineNumber);

  const size_t wordPos = randomLine.find(' ');
  std::string word = randomLine.substr(0, wordPos);
  std::string file = randomLine.substr(wordPos + 1);
  // TODO: save file name in game state
  file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());

  int wordLength = word.length();
  int mistakes = getNumberMistakes(wordLength);

  GameState newGame = createGame(wordLength, mistakes);
  newGame.setWord(word);

  newGame.setSpotsLeft(wordLength);

  GameSessisons.insert(std::pair<std::string, GameState>(plid, newGame));

  arguments = buildSplitString({std::to_string(wordLength), std::to_string(mistakes)});

  return CREATE_GAME_SUCCESS;
}

// TODO: could be better
void setPath(std::string path) {

  // see if the path is valid
  if (access(path.c_str(), F_OK) == -1) {
    std::cerr << "[ERR]: Invalid path." << std::endl;
    return;
  }

  filepath = path;

  std::ifstream file(filepath);

  std::string line;

  totalLines = 0;

  while (std::getline(file, line)) {
    lines.push_back(line);
    totalLines++;
  }

  if (totalLines == 0) {
    std::cout << "File is empty." << std::endl;
  }
}

int isOngoingGame(std::string plid) {
  if (GameSessisons.find(plid) == GameSessisons.end()) {
    // There is no game with this plid
    return 0;
  }
  if (GameSessisons[plid].isActive()) {
    // There is an active game with this plid
    return -1;
  }
  // There is an inactive game with this plid
  return 0;
}

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments) {
  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) == 0) {
    return SYNTAX_ERROR;
  }

  GameState *play = &GameSessisons[plid];

  arguments = play->getTrials();

  if (std::stoi(trial) != play->getTrials()) {
    return TRIAL_MISMATCH;
  }

  if (play->isLetterGuessed(letter[0])) {
    return DUPLICATE_GUESS;
  }

  int numberCorrect = getOccurances(play->getWord(), letter[0], arguments);

  if (numberCorrect == 0) {
    play->incorrectGuess();
    if (play->getAvailableMistakes() == -1) {
      return WRONG_FINAL_GUESS;
    }
    return WRONG_GUESS;
  }

  play->correctGuess(arguments, numberCorrect);

  if (play->getSpotsLeft() == 0) {
    return SUCCESS_FINAL_GUESS;
  }

  return SUCCESS_GUESS;
}

int getOccurances(std::string word, char letter, std::string &positions) {
  int numberCorrect = 0;

  for (size_t i = 0; i < word.length(); i++) {
    if (word[i] == letter) {
      positions += std::to_string(i) + " ";
      numberCorrect++;
    }
  }
  return numberCorrect;
}

int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments) {

  GameState *play = &GameSessisons[plid];
  arguments = play->getTrials();

  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) == 0) {
    return SYNTAX_ERROR;
  }

  arguments = trial;

  if (std::stoi(trial) != play->getTrials() || word == play->getLastWordGuess()) {
    return TRIAL_MISMATCH;
  }

  play->setLastWordGuess(word);

  // TODO: see how to handle win game
  if (play->getWord() == word) {
    return SUCCESS_GUESS;
  }

  play->incorrectGuess();
  if (play->getAvailableMistakes() == -1) {
    return WRONG_FINAL_GUESS;
  }
  return WRONG_GUESS;
}
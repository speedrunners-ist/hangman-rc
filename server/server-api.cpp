#include "server-api.h"

static std::string filepath;
static std::vector<std::string> lines;
static int totalLines;
static std::map<std::string, ServerGameState> GameSessions;

// ServerGameState methods implementation
ServerGameState::ServerGameState() { active = false; }
ServerGameState::ServerGameState(int length, int mistakes) {
  wordLength = length;
  mistakesLeft = mistakes;
  active = true;
  // word is a string with length equal to wordLength, filled with underscores
  word = std::string((size_t)length, '_');
  for (char c = 'a'; c <= 'z'; c++) {
    guessedLetters[c] = false;
  }
}

bool ServerGameState::isActive() { return active; }

int ServerGameState::getAvailableMistakes() { return mistakesLeft; }

char ServerGameState::getLastGuess() { return lastGuess; }

std::string ServerGameState::getLastWordGuess() { return lastWordGuess; }

int ServerGameState::getWordLength() { return wordLength; }

std::string ServerGameState::getWord() { return word; }

void ServerGameState::setLastGuess(char guess) { lastGuess = guess; }

void ServerGameState::setLastWordGuess(std::string guess) { lastWordGuess = guess; }

void ServerGameState::setWord(std::string newWord) { word = newWord; }

bool ServerGameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void ServerGameState::setSpotsLeft(int spots) { spotsLeft = spots; }

int ServerGameState::getSpotsLeft() { return spotsLeft; }

void ServerGameState::incorrectGuess() {
  // TODO: do we have to check if we're setting to true something that's already true?
  guessesMade++;
  mistakesLeft--;
  incrementTrials();
}

int ServerGameState::correctGuess(std::string positions, int n) {
  (void)positions;
  guessesMade++;
  // guessedLetters[guess] = true;
  spotsLeft -= n;
  incrementTrials();

  return 0;
}

void ServerGameState::correctFinalGuess() {
  char guess = getLastGuess();
  guessedLetters[guess] = true;
  // replace all underscores with the guess
  std::replace(word.begin(), word.end(), '_', guess);
  active = false;
}

void ServerGameState::correctFinalWordGuess() {
  word = lastWordGuess;
  active = false;
}

void ServerGameState::incrementTrials() { trials++; }
int ServerGameState::getTrials() { return trials; }

// Game state functions, useful for the client's protocol implementations
ServerGameState createGame(int length, int mistakes) { return ServerGameState(length, mistakes); }
int getAvailableMistakes(ServerGameState play) { return play.getAvailableMistakes(); }
std::string getWord(ServerGameState play) { return play.getWord(); }

int playCorrectGuess(ServerGameState play, std::string positions, int n) {
  int ret = play.correctGuess(positions, n);
  if (ret == 0) {
    play.incrementTrials();
  }
  return ret;
}
void playIncorrectGuess(ServerGameState play) {
  play.incrementTrials();
  play.incorrectGuess();
}
void playCorrectFinalGuess(ServerGameState play) {
  play.incrementTrials();
  play.correctFinalGuess();
}

void playCorrectFinalWordGuess(ServerGameState play) {
  play.incrementTrials();
  play.correctFinalWordGuess();
}

void setLastGuess(ServerGameState play, char guess) { play.setLastGuess(guess); }
void setLastWordGuess(ServerGameState play, std::string guess) { play.setLastWordGuess(guess); }
int getWordLength(ServerGameState play) { return play.getWordLength(); }

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

bool forceExit(ServerGameState play, std::string command) {
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

  // TODO: check this
  ulong randomLineNumber = (ulong)(rand() % totalLines);
  std::string randomLine = lines.at(randomLineNumber);

  const size_t wordPos = randomLine.find(' ');
  std::string word = randomLine.substr(0, wordPos);
  std::string file = randomLine.substr(wordPos + 1);
  // TODO: save file name in game state
  file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());

  // TODO: check this conversion
  int wordLength = (int)word.length();
  int mistakes = getNumberMistakes(wordLength);

  ServerGameState newGame = createGame(wordLength, mistakes);
  newGame.setWord(word);

  newGame.setSpotsLeft(wordLength);

  GameSessions.insert(std::pair<std::string, ServerGameState>(plid, newGame));

  arguments.append(std::to_string(wordLength)).append(" ").append(std::to_string(mistakes));

  return CREATE_GAME_SUCCESS;
}

// TODO: could be better
int setPath(std::string path) {

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
    return -1;
  }

  return 0;
}

int isOngoingGame(std::string plid) {
  if (GameSessions.find(plid) == GameSessions.end()) {
    // There is no game with this plid
    return 0;
  }
  return GameSessions[plid].isActive() ? -1 : 0;
}

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments) {
  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) == 0) {
    return SYNTAX_ERROR;
  }

  ServerGameState *play = &GameSessions[plid];

  arguments = std::to_string(play->getTrials());

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
  std::string auxiliar = "";

  for (size_t i = 0; i < word.length(); i++) {
    if (word[i] == letter) {
      auxiliar.append(" ").append(std::to_string(i));
      numberCorrect++;
    }
  }
  positions.append(" ").append(std::to_string(numberCorrect)).append(auxiliar);

  return numberCorrect;
}

int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments) {

  ServerGameState *play = &GameSessions[plid];
  arguments = std::to_string(play->getTrials());

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

int closeGameSession(std::string plid) {
  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) == 0) {
    return CLOSE_GAME_ERROR;
  }

  GameSessions.erase(plid);
  return CLOSE_GAME_SUCCESS;
}

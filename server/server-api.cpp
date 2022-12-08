#include "server-api.h"

// wordlist file
std::string filepath;
// game state file
std::string fileName;
std::vector<std::string> lines;
int totalLines;
std::map<std::string, GameState> GameSessions;

/*** GameState implementation ***/

bool GameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void GameState::setSpotsLeft(int spots) { spotsLeft = spots; }

int GameState::getSpotsLeft() { return spotsLeft; }

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
int createGameSession(std::string plid, std::string &arguments) {

  if (validatePlayerID(plid) != 0) {
    return CREATE_GAME_ERROR;
  }

  if (isOngoingGame(plid) == 1) {
    std::cout << "Game already exists for player " << plid << std::endl;
    if (isGamePlayed() == -1) {
      return CREATE_GAME_ERROR;
    }
  }

  // TODO: check this
  ulong randomLineNumber = (ulong)(rand() % totalLines);
  std::string randomLine = lines.at(randomLineNumber);

  const size_t wordPos = randomLine.find(' ');
  std::string word = randomLine.substr(0, wordPos);
  std::string file = randomLine.substr(wordPos + 1);
  // TODO: save file name in game state
  file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());

  const int wordLength = (int)word.length();
  const int mistakes = initialAvailableMistakes(wordLength);

  GameState newGame = createGame(wordLength, mistakes);
  newGame.setWord(word);

  newGame.setSpotsLeft(wordLength);

  GameSessions.insert(std::pair<std::string, GameState>(plid, newGame));

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

  fileName = "";
  return findOccurringGame((char *)plid.c_str(), (char *)fileName.c_str()) == 0 ? 0 : 1;
}

// TODO: check better
int checkHeader(std::string line) {
  std::string word = "";
  word = line.substr(0, line.find(' '));
  if (word == "")
    return -1;
  line.erase(0, line.find(' ') + 1);
  word = "";
  word = line.substr(0, line.find('\n'));
  if (word == "")
    return -1;

  // TODO: check file exists
  return 0;
}

int isGamePlayed() {

  std::fstream file;
  std::string line;

  file.open(fileName, std::ios::in);
  if (!file) {
    return -1;
  }

  // see first line
  std::getline(file, line);
  if (checkHeader(line) == -1) {
    file.close();
    return -1;
  }

  // see second line
  line = "";
  std::getline(file, line);
  file.close();
  return line == "" ? 0 : -1;
}

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments) {
  if (validatePlayerID(plid) != 0) {
    return SYNTAX_ERROR;
  }

  // see if file is empty
  if (isOngoingGame(plid) == 1) {

    fflush(stdout);
    // see if file has actions
    if (isGamePlayed() == -1) {
      return SYNTAX_ERROR;
    }
  }

  GameState *play = &GameSessions[plid];

  arguments = std::to_string(play->getTrials());

  if (std::stoi(trial) != play->getTrials()) {
    return TRIAL_MISMATCH;
  }

  if (play->isLetterGuessed(letter[0])) {
    return DUPLICATE_GUESS;
  }

  int numberCorrect = getOccurrences(play->getWord(), letter[0], arguments);

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

int getOccurrences(std::string word, char letter, std::string &positions) {
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

  GameState *play = &GameSessions[plid];
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

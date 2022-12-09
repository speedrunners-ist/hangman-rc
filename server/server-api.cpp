#include "server-api.h"

std::string filepath;
std::vector<std::string> lines;
int totalLines;
std::map<std::string, GameState> GameSessions;

/*** GameState implementation ***/

bool GameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void GameState::setSpotsLeft(int spots) { spotsLeft = spots; }

int GameState::getSpotsLeft() { return spotsLeft; }

// Game state functions, useful for the client's protocol implementations
GameState createGame(int length, int mistakes, std::string playerID) {
  return GameState(length, mistakes, playerID);
}
int getAvailableMistakes(GameState play) { return play.getAvailableMistakes(); }
std::string getWord(GameState play) { return play.getWord(); }

int playCorrectGuess(GameState &play, std::string positions, int n) {
  int ret = play.correctGuess(positions, n);
  if (ret == 0) {
    play.incrementTrials();
  }
  return ret;
}
void playIncorrectGuess(GameState &play) {
  play.incrementTrials();
  play.incorrectGuess();
}
void playCorrectFinalGuess(GameState &play) {
  play.incrementTrials();
  play.correctFinalGuess();
}

void playCorrectFinalWordGuess(GameState &play) {
  play.incrementTrials();
  play.correctFinalWordGuess();
}

void setLastGuess(GameState &play, char guess) { play.setLastGuess(guess); }
void setLastWordGuess(GameState &play, std::string guess) { play.setLastWordGuess(guess); }
int getWordLength(GameState play) { return play.getWordLength(); }
void setPlayerID(GameState &play, std::string id) { play.setPlayerID(id); }
std::string getPlayerID(GameState play) { return play.getPlayerID(); }
int getTrials(GameState play) {
  // the user will always send the trial number related to the one he is playing, hence the +1
  return play.getTrials() + 1;
}
void incrementTrials(GameState &play) { play.incrementTrials(); }

// Util functions
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

  const int wordLength = (int)word.length();
  const int mistakes = initialAvailableMistakes(wordLength);

  GameState newGame = createGame(wordLength, mistakes, plid);
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

  GameState *play = &GameSessions[plid];

  arguments = std::to_string(getTrials(*play));

  std::cout << "Received trial " << trial << " and current trial is " << getTrials(*play) << std::endl;

  if (std::stoi(trial) != getTrials(*play)) {
    return TRIAL_MISMATCH;
  }

  if (play->isLetterGuessed(letter[0])) {
    return DUPLICATE_GUESS;
  }

  incrementTrials(*play);
  std::cout << "Incremented trials to " << getTrials(*play) << std::endl;

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
  arguments = std::to_string(getTrials(*play));

  if (validatePlayerID(plid) != 0 || isOngoingGame(plid) == 0) {
    return SYNTAX_ERROR;
  }

  arguments = trial;

  if (std::stoi(trial) != getTrials(*play) || word == play->getLastWordGuess()) {
    return TRIAL_MISMATCH;
  }

  incrementTrials(*play);
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

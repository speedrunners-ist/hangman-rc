#include "server-api.h"

// wordlist file
std::string filepath;
// game state file
std::string fileName;
std::vector<std::string> wordsList;
int totalLines;
std::map<std::string, GameState> GameSessions;

/*** GameState implementation ***/

bool GameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void GameState::setSpotsLeft(int spots) { spotsLeft = spots; }

int GameState::getSpotsLeft() { return spotsLeft; }

void GameState::setHint(std::string newHint) { hint = newHint; }

std::string GameState::getHint() { return hint; }

void GameState::addGuessedLetter(char letter) { guessedLetters[letter] = true; }

void GameState::addGuessedWord(std::string guessedWord) { guessedWords[guessedWord] = true; }

void GameState::setMistakesLeft(int mistakes) { mistakesLeft = mistakes; }

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

int readFile(std::vector<std::string> &lines) {

  std::fstream file;
  std::string line = "";
  std::string word = "";
  std::string hint = "";

  file.open(fileName, std::ios::in);
  if (!file) {
    return -1;
  }

  while (getline(file, line)) {
    lines.push_back(line);
  }

  file.close();

  return 0;
}

// Util functions
int createGameSession(std::string plid, std::string &arguments) {

  if (validatePlayerID(plid) != 0) {
    return CREATE_GAME_ERROR;
  }

  // if game is already ongoing
  if (isOngoingGame(plid) == 1) {

    GameState gamestate;
    createGameState(gamestate);

    // if game has moves
    if (gamestate.getTrials() != 1) {
      return CREATE_GAME_ERROR;
    }

    std::string word = gamestate.getWord();
    arguments = buildSplitString({std::to_string(word.length()),
                                  std::to_string(initialAvailableMistakes((int)word.length()))});

    return CREATE_GAME_SUCCESS;
  }

  // TODO: check this
  ulong randomLineNumber = (ulong)(rand() % totalLines);
  std::string randomLine = wordsList.at(randomLineNumber);

  const size_t wordPos = randomLine.find(' ');
  std::string word = randomLine.substr(0, wordPos);
  std::string file = randomLine.substr(wordPos + 1);
  // TODO: save file name in game state
  file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());

  arguments = buildSplitString({std::to_string(word.length()),
                                std::to_string(initialAvailableMistakes((int)word.length()))});
  createGameFile(plid, word, file);

  return CREATE_GAME_SUCCESS;
}

// TODO: could be better
int setPath(std::string path) {

  filepath = path;
  std::ifstream file(filepath);
  std::string line;

  totalLines = 0;

  while (std::getline(file, line)) {
    wordsList.push_back(line);
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

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments) {
  if (validatePlayerID(plid) != 0) {
    return SYNTAX_ERROR;
  }

  // see if file is empty
  if (isOngoingGame(plid) != 1) {
    return SYNTAX_ERROR;
  }

  GameState gamestate;
  createGameState(gamestate);

  arguments = std::to_string(gamestate.getTrials());

  // if trial mismatch
  if (std::stoi(trial) != gamestate.getTrials()) {
    if (std::stoi(trial) != gamestate.getTrials() - 1)
      return TRIAL_MISMATCH;

    if (gamestate.getLastGuess() != letter[0]) {
      return TRIAL_MISMATCH;
    }

      // todo: FIX THIS
    // return ;
  }

  if (gamestate.isLetterGuessed(letter[0])) {
    return DUPLICATE_GUESS;
  }

  int numberCorrect = getOccurrences(gamestate.getWord(), letter[0], arguments);

  appendGameFile(plid, "T", letter);

  if (numberCorrect == 0) {
    gamestate.incorrectGuess();
    if (gamestate.getAvailableMistakes() == -1) {
      return WRONG_FINAL_GUESS;
    }
    return WRONG_GUESS;
  }

  gamestate.correctGuess(arguments, numberCorrect);

  if (gamestate.getSpotsLeft() == 0) {
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
  if (numberCorrect > 0)
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

int createGameState(GameState &gamestate) {
  std::vector<std::string> lines;

  if (readFile(lines) == -1) {
    return -1;
  }

  std::string header = lines[0];

  std::string word = header.substr(0, header.find(' '));
  header.erase(0, header.find(' ') + 1);
  std::string hint = header;

  gamestate.setWord(word);
  gamestate.setMistakesLeft(initialAvailableMistakes((int)word.length()));
  gamestate.setHint(hint);
  // TODO: check if this is correct
  gamestate.incrementTrials();

  for (size_t i = 1; i < lines.size(); i++) {
    std::string line = lines[i];
    std::string code = line.substr(0, line.find(' '));
    line.erase(0, line.find(' ') + 1);
    std::string play = line;
    // Trial
    if (code == "T") {
      gamestate.addGuessedLetter(play[0]);
      gamestate.setLastGuess(play[0]);
      gamestate.incrementTrials();

    }
    // Guess word
    else if (code == "G") {
      gamestate.addGuessedWord(play);
      gamestate.setLastWordGuess(play);
      gamestate.incrementTrials();
    }

    else {
      std::cout << code << std::endl;
      std::cout << "Error: invalid code." << std::endl;
    }
  }
  return 0;
}
#include "server-api.h"

std::map<std::string, std::string> wordsList;
int totalLines;

// TODO: either call everything GameState play or GameState state, but not both

/*** GameState implementation ***/

bool GameState::isLetterGuessed(char letter) { return guessedLetters[letter]; }

void GameState::setSpotsLeft(int spots) { spotsLeft = spots; }

int GameState::getSpotsLeft() { return spotsLeft; }

void GameState::setHint(std::string newHint) { hint = newHint; }

std::string GameState::getHint() { return hint; }

void GameState::addGuessedLetter(char letter) { guessedLetters[letter] = true; }

void GameState::addGuessedWord(std::string guessedWord) { guessedWords[guessedWord] = true; }

void GameState::setMistakesLeft(int mistakes) { mistakesLeft = mistakes; }

/*** Util methods in order to let external server implementations use the GameState class ***/
int getAvailableMistakes(GameState play) { return play.getAvailableMistakes(); }
std::string getWord(GameState play) { return play.getWord(); }
void setLastGuess(GameState play, char guess) { play.setLastGuess(guess); }
void setLastWordGuess(GameState play, std::string guess) { play.setLastWordGuess(guess); }
int getWordLength(GameState play) { return play.getWordLength(); }
void setPlayerID(GameState &play, std::string id) { play.setPlayerID(id); }
std::string getPlayerID(GameState play) { return play.getPlayerID(); }
int getTrials(GameState play) {
  // the user will always send the trial number related to the one he is playing, hence the +1
  return play.getTrials() + 1;
}
void incrementTrials(GameState &play) { play.incrementTrials(); }

/*** General util methods ***/
int setupWordList(std::string filePath) {
  std::vector<std::string> lines;
  readFile(lines, filePath);
  if (lines.size() == 0) {
    std::cerr << "[ERR]: File " << filePath << " is empty." << std::endl;
    return -1;
  }

  for (auto line : lines) {
    const size_t wordPos = line.find(' ');
    std::string word = line.substr(0, wordPos);
    std::string file = line.substr(wordPos + 1);
    wordsList[word] = file;
  }
  return 0;
}

bool isOngoingGame(std::string plid) { return std::filesystem::exists(ONGOING_GAMES_PATH(plid)); }

std::pair<std::string, std::string> getRandomLine() {
  const int randomIndex = rand() % (int)wordsList.size();
  auto it = wordsList.begin();
  std::advance(it, randomIndex);
  return *it;
}

int getLetterOccurrences(std::string word, char letter, std::string &positions) {
  int occurrences = 0;

  for (size_t i = 1; i <= word.length(); i++) {
    if (word[i] == letter) {
      positions.append(std::to_string(i) + " ");
      occurrences++;
    }
  }
  positions.insert(0, std::to_string(occurrences) + " ");
  return occurrences;
}

int createGameSession(std::string plid, std::string &arguments) {
  if (!validPlayerID(plid)) {
    return CREATE_GAME_ERROR;
  }

  // If there's already an ongoing game for the given player
  if (isOngoingGame(plid)) {
    GameState state;
    if (retrieveGame(plid, state) != 0) {
      return CREATE_GAME_ERROR;
    }

    // FIXME: what the duck is this? shouldn't we just let the player continue the game?
    if (state.getTrials() != 1) {
      return CREATE_GAME_ERROR;
    }

    const std::string word = state.getWord();
    const int wordLength = (int)word.length();
    const int availableMistakes = initialAvailableMistakes(wordLength);

    arguments = buildSplitString({std::to_string(wordLength), std::to_string(availableMistakes)});
    return CREATE_GAME_SUCCESS;
  }

  // Here, we must create a new game (since the player either never played a game
  // or finished the last one)
  // We pick a random line from the words list for the game
  const std::pair<std::string, std::string> randomLine = getRandomLine();
  const std::string word = randomLine.first;
  const std::string hint = randomLine.second;
  const int wordLength = (int)word.length();
  const int availableMistakes = initialAvailableMistakes(wordLength);

  arguments = buildSplitString({std::to_string(wordLength), std::to_string(availableMistakes)});
  if (createGameFile(plid, word, hint) != 0) {
    return CREATE_GAME_ERROR;
  }
  GameState state = GameState(wordLength, availableMistakes, plid);
  state.setWord(word);
  state.setHint(hint);
  return CREATE_GAME_SUCCESS;
}

int retrieveGame(std::string playerID, GameState &state) {
  // Retrieve the content from its file
  std::vector<std::string> lines;
  const std::string stateFilePath = ONGOING_GAMES_PATH(playerID);
  if (readFile(lines, stateFilePath) != 0) {
    return -1;
  }

  // We parse the file content and create a GameState object from it
  // TODO: parse file content

  return 0;
}

int playLetter(std::string plid, std::string letter, std::string trial, std::string &positions) {
  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    // FIXME: I don't think the name "SYNTAX_ERROR" is correct...
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return SYNTAX_ERROR;
  } else if (std::stoi(trial) != state.getTrials()) {
    return TRIAL_MISMATCH;
  } else if (state.isLetterGuessed(letter.front())) {
    return DUPLICATE_GUESS;
  }

  const int occurrences = getLetterOccurrences(state.getWord(), letter.front(), positions);
  appendGameFile(plid, "T", letter);

  if (occurrences == 0) {
    state.setMistakesLeft(state.getAvailableMistakes() - 1);
    if (state.getAvailableMistakes() == -1) {
      appendGameFile(plid, "L", letter);
      if (transferGameFile(plid) != 0) {
        return -1; // TODO: specific macro for this
      }
      return WRONG_FINAL_GUESS;
    }
    return WRONG_GUESS;
  }

  state.setSpotsLeft(state.getSpotsLeft() - occurrences);

  if (state.getSpotsLeft() == 0) {
    appendGameFile(plid, "W", letter);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    insertScore(plid, state);
    return SUCCESS_FINAL_GUESS;
  }

  return SUCCESS_GUESS;
}

int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments) {
  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return SYNTAX_ERROR;
  } else if (std::stoi(trial) != state.getTrials()) {
    return TRIAL_MISMATCH;
  }

  arguments = trial;

  state.setLastWordGuess(word);
  state.addGuessedWord(word);
  state.incrementTrials();

  appendGameFile(plid, "G", word);

  if (state.getWord() == word) {
    appendGameFile(plid, "W", word);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    insertScore(plid, state);
    return SUCCESS_GUESS;
  }

  state.setMistakesLeft(state.getAvailableMistakes() - 1);
  if (state.getAvailableMistakes() == -1) {
    appendGameFile(plid, "L", word);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    return WRONG_FINAL_GUESS;
  }
  return WRONG_GUESS;
}

int closeGameSession(std::string plid) {
  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    return CLOSE_GAME_ERROR;
  }

  appendGameFile(plid, "Q", "");
  transferGameFile(plid);
  return CLOSE_GAME_SUCCESS;
}

int insertScore(std::string plid, GameState state) {
  const int initialMistakes = initialAvailableMistakes(getWordLength(state));
  const int trialsMade = state.getTrials() - 1;
  const int successfulGuesses = trialsMade - (initialMistakes - state.getAvailableMistakes());
  const int score = GAME_SCORE(successfulGuesses, trialsMade);

  std::cout << "Successful guesses: " << successfulGuesses << std::endl;
  std::cout << "Total guesses: " << trialsMade << std::endl;
  std::cout << "Score: " << score << std::endl;

  // printedScore is the score with 3 digits (i.e, if it was 9, it's 009, etc)
  std::stringstream scoreStream;
  scoreStream << std::setw(3) << std::setfill('0') << score;
  const std::string printedScore = scoreStream.str();

  // clang-format off
  std::string scoreline = buildSplitString({
    printedScore,
    plid,
    state.getWord(),
    std::to_string(successfulGuesses),
    std::to_string(trialsMade)
  });
  // clang-format on

  appendScoreFile(score, scoreline);
  return score;
}

#include "server-api.h"

std::map<std::string, std::string> wordsList;

// clang-format off
// Note that there's no need to handle correct guesses, since in that case
// the game would not be ongoing anymore
std::map<std::string, std::function<void(GameState &state, std::string value)>> handleLineRetrieval = {
  {CORRECT_LETTER, playCorrectLetterGuess},
  {WRONG_LETTER, playIncorrectLetterGuess},
  {WRONG_WORD, playIncorrectWordGuess}
};
// clang-format on

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

int getAvailableMistakes(GameState state) { return state.getAvailableMistakes(); }

std::string getWord(GameState state) { return state.getWord(); }

int getWordLength(GameState state) { return state.getWordLength(); }

void setPlayerID(GameState &state, std::string plid) { state.setPlayerID(plid); }

std::string getPlayerID(GameState state) { return state.getPlayerID(); }

int getTrials(GameState state) {
  // the user will always send the trial number related to the one he is playing, hence the +1
  return state.getTrials() + 1;
}
void incrementTrials(GameState &state) { state.incrementTrials(); }

/*** Methods utilized in state file retrieval ***/

void playCorrectLetterGuess(GameState &state, std::string letter) {
  const int occurrences = getLetterOccurrences(state.getWord(), letter.front());
  state.setSpotsLeft(state.getSpotsLeft() - occurrences);
  state.incrementTrials();
}

void playIncorrectLetterGuess(GameState &state, std::string letter) {
  state.setMistakesLeft(state.getAvailableMistakes() - 1);
  state.addGuessedLetter(letter.front());
  state.incrementTrials();
}

void playIncorrectWordGuess(GameState &state, std::string word) {
  state.setMistakesLeft(state.getAvailableMistakes() - 1);
  state.addGuessedWord(word);
  state.incrementTrials();
}

/*** General util methods ***/

int setupWordList(std::string filePath) {
  std::vector<std::string> lines;
  readFile(lines, filePath);
  if (lines.size() == 0) {
    std::cerr << EMPTY_FILE(filePath) << std::endl;
    return -1;
  }

  for (auto line : lines) {
    const size_t wordPos = line.find(' ');
    std::string word = line.substr(0, wordPos);
    toLower(word);
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

int getLetterOccurrences(std::string word, char letter) {
  return (int)std::count(word.begin(), word.end(), letter);
}

int getLetterOccurrencesPositions(std::string word, char letter, std::string &positions) {
  int occurrences = 0;
  const std::string space = " ";
  for (size_t i = 0; i < word.length(); i++) {
    if (tolower(word[i]) == letter) {
      positions.append(space + std::to_string(i + 1));
      occurrences++;
    }
  }
  positions.insert(0, std::to_string(occurrences));
  return occurrences;
}

int retrieveGame(std::string playerID, GameState &state) {
  // Retrieve the content from its file
  std::vector<std::string> lines;
  const std::string stateFilePath = ONGOING_GAMES_PATH(playerID);
  if (readFile(lines, stateFilePath) != 0) {
    return -1;
  }

  // We parse the file content and create a GameState object from it
  std::string word, hint;
  const std::string firstLine = lines[0];
  const size_t wordPos = firstLine.find(' ');
  word = firstLine.substr(0, wordPos);
  hint = firstLine.substr(wordPos + 1);
  hint.erase(hint.length());

  const int wordLength = (int)word.length();
  const int availableMistakes = initialAvailableMistakes(wordLength);
  state = GameState(wordLength, availableMistakes, playerID);
  state.setWord(word);
  state.setHint(hint);
  state.setSpotsLeft(wordLength);

  lines.erase(lines.begin()); // drop the first line, as it was already parsed
  for (auto l : lines) {
    const std::string key = l.substr(0, l.find(':') + 1);
    const std::string value = l.substr(l.find(':') + 2, l.find('\n'));
    try {
      handleLineRetrieval[key](state, value);
    } catch (const std::out_of_range &e) {
      std::cerr << UNEXPECTED_GAME_LINE(l) << std::endl;
      return -1;
    }
  }
  return 0;
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

    // FIXME: I think we should just allow the player to continue playing the
    // game, even if they have already made a play, hence this being commented out
    // if (state.getTrials() != 1) {
    //   return CREATE_GAME_ERROR;
    // }
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
  state.setSpotsLeft(wordLength);
  return CREATE_GAME_SUCCESS;
}

int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments) {

  toLower(letter);

  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    // FIXME: I don't think the name "SYNTAX_ERROR" is correct...
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    arguments = buildSplitString({std::to_string(getTrials(state))});
    return SYNTAX_ERROR;
  }

  arguments = buildSplitString({std::to_string(getTrials(state))});
  if (std::stoi(trial) != getTrials(state)) {
    return TRIAL_MISMATCH;
  } else if (state.isLetterGuessed(letter.front())) {
    return DUPLICATE_GUESS;
  }

  std::string positions;
  const int occurrences = getLetterOccurrencesPositions(state.getWord(), letter.front(), positions);
  state.addGuessedLetter(letter.front());
  state.incrementTrials();
  arguments = buildSplitString({arguments, positions});

  if (occurrences == 0) {
    state.setMistakesLeft(state.getAvailableMistakes() - 1);
    if (state.getAvailableMistakes() == -1) {
      appendGameFile(plid, WRONG_FINAL_LETTER, letter);
      if (transferGameFile(plid) != 0) {
        return -1; // TODO: specific macro for this
      }
      return WRONG_FINAL_GUESS;
    }
    appendGameFile(plid, WRONG_LETTER, letter);
    return WRONG_GUESS;
  }

  state.setSpotsLeft(state.getSpotsLeft() - occurrences);
  if (state.getSpotsLeft() == 0) {
    appendGameFile(plid, CORRECT_FINAL_LETTER, letter);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    insertScore(plid, state);
    return SUCCESS_FINAL_GUESS;
  }

  appendGameFile(plid, CORRECT_LETTER, letter);
  return SUCCESS_GUESS;
}

int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments) {

  toLower(word);

  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return SYNTAX_ERROR;
    arguments = buildSplitString({std::to_string(getTrials(state))});
  }

  arguments = buildSplitString({std::to_string(getTrials(state))});
  if (std::stoi(trial) != getTrials(state)) {
    return TRIAL_MISMATCH;
  }

  arguments = trial;

  state.setLastWordGuess(word);
  state.addGuessedWord(word);
  state.incrementTrials();

  if (state.getWord() == word) {
    appendGameFile(plid, CORRECT_FINAL_WORD, word);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    insertScore(plid, state);
    return SUCCESS_GUESS;
  }

  state.setMistakesLeft(state.getAvailableMistakes() - 1);
  if (state.getAvailableMistakes() == -1) {
    appendGameFile(plid, WRONG_FINAL_WORD, word);
    if (transferGameFile(plid) != 0) {
      return -1; // TODO: specific macro for this
    }
    return WRONG_FINAL_GUESS;
  }

  appendGameFile(plid, WRONG_WORD, word);
  return WRONG_GUESS;
}

int closeGameSession(std::string plid) {
  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    return CLOSE_GAME_ERROR;
  }

  appendGameFile(plid, QUIT_GAME, "");
  transferGameFile(plid);
  return CLOSE_GAME_SUCCESS;
}

int insertScore(std::string plid, GameState &state) {
  const int initialMistakes = initialAvailableMistakes(getWordLength(state));
  const int trialsMade = state.getTrials();
  const int successfulGuesses = trialsMade - (initialMistakes - state.getAvailableMistakes());
  const int score = GAME_SCORE(successfulGuesses, trialsMade);

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

int getScoreboard(std::string &response) {
  std::vector<std::string> lines;
  int ret = readFile(lines, SCORES_PATH);

  if (ret == -1) {
    return SCOREBOARD_ERROR;
  } else if (ret == -2 || lines.empty()) {
    // if the file did not exist or was empty
    response = "EMPTY";
    return SCOREBOARD_EMPTY;
  }

  long fileSize = (long)std::filesystem::file_size(SCORES_PATH);
  response = buildSplitString({"scoreboard.txt", std::to_string(fileSize)});
  return SCOREBOARD_SUCCESS;
}

int getHint(std::string plid, std::string &response, std::string &filePath) {
  if (!validPlayerID(plid) || !isOngoingGame(plid)) {
    return HINT_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return HINT_ERROR;
  }

  const std::string fileName = state.getHint();
  filePath = HINTS_PATH(fileName);
  std::vector<std::string> lines;
  if (readFile(lines, filePath) != 0) {
    return HINT_ERROR;
  }

  long fileSize = (long)std::filesystem::file_size(filePath);
  response = buildSplitString({std::string(fileName), std::to_string(fileSize)});
  return HINT_SUCCESS;
}

int getState(std::string plid, std::string &response, std::string &filePath) {
  if (!validPlayerID(plid)) {
    return STATE_ERROR;
  }

  const bool isFinished = !isOngoingGame(plid);
  std::string mostRecentGame;

  if (isFinished) {
    int ret = getLastFinishedGame(plid, mostRecentGame);
    if (ret != 0) {
      return STATE_ERROR;
    }
    filePath = PLID_GAMES_DIR(plid) + "/" + mostRecentGame;
  } else {
    filePath = ONGOING_GAMES_PATH(plid);
    int ret = createPlaceholderState(plid, filePath);
    if (ret != 0) {
      return STATE_ERROR;
    }
  }

  const std::string fileName = std::filesystem::path(filePath).filename();
  std::vector<std::string> lines;
  if (readFile(lines, filePath) != 0) {
    return STATE_ERROR;
  }

  if (!isFinished) {
    filePath = TMP_PATH(plid);
  }

  size_t fileSize = std::filesystem::file_size(filePath);
  response = buildSplitString({fileName, std::to_string(fileSize)});
  return isFinished ? STATE_FINISHED : STATE_ONGOING;
}

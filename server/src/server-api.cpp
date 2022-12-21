#include "server-api.h"

std::vector<std::pair<std::string, std::string>> wordsList;
std::vector<std::pair<std::string, std::string>>::iterator currentPair;

// clang-format off
std::map<std::string, std::function<void(GameState &state, std::string value)>> handleLineRetrieval = {
  {CORRECT_LETTER, playCorrectLetterGuess},
  {WRONG_LETTER, playIncorrectLetterGuess},
  {WRONG_WORD, playIncorrectWordGuess},
  {HINT, setHint},
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

bool GameState::isWordGuessed(std::string guessedWord) { return guessedWords[guessedWord]; }

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
  state.addGuessedLetter(letter.front());
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

void setHint(GameState &state, std::string hint) { state.setHint(hint); }

/*** General util methods ***/

void displayPeerInfo(struct addrinfo *res, std::string connection) {
  char host[NI_MAXHOST] = {0};
  char service[NI_MAXSERV] = {0};
  const int errcode =
      getnameinfo(res->ai_addr, res->ai_addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
  // NI_NUMERICSERV: return numeric form of the service's address
  if (errcode != 0) {
    std::cerr << VERBOSE_ERROR(errcode) << std::endl;
    return;
  }

  std::cout << VERBOSE_SUCCESS(connection, host, service) << std::endl;
}

int setupWordList(std::string filePath) {
  std::vector<std::string> lines;
  if (readFile(lines, filePath) < 0) {
    return -1;
  }
  if (lines.size() == 0) {
    std::cerr << EMPTY_FILE(filePath) << std::endl;
    return -1;
  }

  for (auto line : lines) {
    const size_t wordPos = line.find(' ');
    std::string word = line.substr(0, wordPos);
    toLower(word);
    const std::string hint = line.substr(wordPos + 1);
    wordsList.push_back(std::make_pair(word, hint));
  }
  currentPair = wordsList.begin(); // The first word will be the first one in the list
  return 0;
}

bool isOngoingGame(std::string plid) { return std::filesystem::exists(ONGOING_GAMES_PATH(plid)); }

bool isInvalidTrial(std::string trial) {
  try {
    const int t = std::stoi(trial);
    return t <= 0 || t > 100;
  } catch (const std::invalid_argument &e) {
    return true;
  }
}

std::pair<std::string, std::string> getWordHintPair() {
#ifdef PRODUCTION
  // Random in production mode
  const int randomIndex = rand() % (int)wordsList.size();
  currentPair = wordsList.begin();
  std::advance(currentPair, randomIndex);
  return *currentPair;
#endif
  // Sequential in development mode
  if (currentPair == wordsList.end()) {
    currentPair = wordsList.begin();
  }
  return *currentPair++;
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
    } catch (const std::bad_function_call &e) {
      std::cerr << UNEXPECTED_GAME_LINE(l) << std::endl;
      return -1;
    }
  }
  return 0;
}

int createGameSession(std::string plid, std::string &arguments) {
  // If there's already an ongoing game for the given player
  if (isOngoingGame(plid)) {
    GameState state;
    if (retrieveGame(plid, state) != 0) {
      return CREATE_GAME_ERROR;
    }

    if (state.getTrials() != 1) {
      // If a game is ongoing, the player is not allowed to start a new one
      return CREATE_GAME_ONGOING;
    }
    const std::string word = state.getWord();
    const int wordLength = (int)word.length();
    const int availableMistakes = initialAvailableMistakes(wordLength);

    arguments = buildSplitString({std::to_string(wordLength), std::to_string(availableMistakes)});
    return CREATE_GAME_SUCCESS;
  }

  // Here, we must create a new game (since the player either never played a game
  // or finished the last one)
  const std::pair<std::string, std::string> line = getWordHintPair();
  const std::string word = line.first;
  const std::string hint = line.second;
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
  if (!isOngoingGame(plid)) {
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return SYNTAX_ERROR;
  }

  arguments = buildSplitString({std::to_string(getTrials(state) - 1)});

  if (std::stoi(trial) != getTrials(state)) {
    return TRIAL_MISMATCH;
  }

  toLower(letter);
  if (state.isLetterGuessed(letter.front())) {
    return DUPLICATE_GUESS;
  }

  std::string positions;
  const int occurrences = getLetterOccurrencesPositions(state.getWord(), letter.front(), positions);
  state.addGuessedLetter(letter.front());
  state.incrementTrials();
  arguments = buildSplitString({std::to_string(getTrials(state) - 1)});

  if (occurrences == 0) {
    state.setMistakesLeft(state.getAvailableMistakes() - 1);
    if (state.getAvailableMistakes() == -1) {
      appendGameFile(plid, WRONG_FINAL_LETTER, letter);
      if (transferGameFile(plid) != 0) {
        return GENERAL_ERROR;
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
      return GENERAL_ERROR;
    }
    insertScore(plid, state);
    return SUCCESS_FINAL_GUESS;
  }
  arguments = buildSplitString({arguments, positions});

  appendGameFile(plid, CORRECT_LETTER, letter);
  return SUCCESS_GUESS;
}

int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments) {
  if (!isOngoingGame(plid)) {
    return SYNTAX_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return SYNTAX_ERROR;
  }
  arguments = buildSplitString({std::to_string(getTrials(state) - 1)});

  if (std::stoi(trial) != getTrials(state)) {
    return TRIAL_MISMATCH;
  }

  toLower(word);
  if (state.isWordGuessed(word)) {
    return DUPLICATE_GUESS;
  }

  state.setLastWordGuess(word);
  state.addGuessedWord(word);
  state.incrementTrials();
  arguments = buildSplitString({std::to_string(getTrials(state) - 1)});

  if (state.getWord() == word) {
    appendGameFile(plid, CORRECT_FINAL_WORD, word);
    if (transferGameFile(plid) != 0) {
      return GENERAL_ERROR;
    }
    insertScore(plid, state);
    return SUCCESS_GUESS;
  }

  state.setMistakesLeft(state.getAvailableMistakes() - 1);
  if (state.getAvailableMistakes() == -1) {
    appendGameFile(plid, WRONG_FINAL_WORD, word);
    if (transferGameFile(plid) != 0) {
      return GENERAL_ERROR;
    }
    return WRONG_FINAL_GUESS;
  }

  appendGameFile(plid, WRONG_WORD, word);
  return WRONG_GUESS;
}

int closeGameSession(std::string plid) {
  if (!isOngoingGame(plid)) {
    return CLOSE_GAME_ERROR;
  }

  appendGameFile(plid, QUIT_GAME, "");
  transferGameFile(plid);
  return CLOSE_GAME_SUCCESS;
}

int revealWord(std::string plid, std::string &word) {
  if (!isOngoingGame(plid)) {
    return REVEAL_ERROR;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return REVEAL_ERROR;
  }

  word = RRV_OK(state.getWord());
  return REVEAL_SUCCESS;
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
    "    ",
    plid,
    "",
    std::to_string(successfulGuesses),
    "           ",
    std::to_string(trialsMade),
    "      ",
    state.getWord(),
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
  }
  if (ret == -2 || lines.empty()) {
    // if the file did not exist or was empty
    response = "EMPTY";
    return SCOREBOARD_EMPTY;
  }

  long fileSize = (long)std::filesystem::file_size(SCORES_PATH);
  response = buildSplitString({"scoreboard.txt", std::to_string(fileSize)});
  return SCOREBOARD_SUCCESS;
}

int getHint(std::string plid, std::string &response, std::string &filePath) {
  if (!isOngoingGame(plid)) {
    return HINT_NOK;
  }

  GameState state;
  if (retrieveGame(plid, state) != 0) {
    return HINT_NOK;
  }

  const std::string fileName = state.getHint();
  filePath = HINTS_PATH(fileName);
  std::vector<std::string> lines;
  if (readFile(lines, filePath) != 0) {
    return HINT_NOK;
  }

  long fileSize = (long)std::filesystem::file_size(filePath);
  response = buildSplitString({std::string(fileName), std::to_string(fileSize)});
  return HINT_SUCCESS;
}

int getState(std::string plid, std::string &response, std::string &filePath) {
  const bool isFinished = !isOngoingGame(plid);
  std::string mostRecentGame;

  if (isFinished) {
    if (getLastFinishedGame(plid, mostRecentGame) != 0) {
      return STATE_NOK;
    }
    filePath = PLID_GAMES_DIR(plid) + "/" + mostRecentGame;
  } else {
    filePath = ONGOING_GAMES_PATH(plid);
    if (createPlaceholderState(plid, filePath) != 0) {
      return STATE_NOK;
    }
  }

  std::vector<std::string> lines;
  if (readFile(lines, filePath) != 0) {
    return STATE_NOK;
  }

  if (!isFinished) {
    filePath = TMP_PATH(plid);
  }

  size_t fileSize = std::filesystem::file_size(filePath);
  const std::string fileName = GAME_FILENAME(plid);

  response = buildSplitString({fileName, std::to_string(fileSize)});
  return isFinished ? STATE_FINISHED : STATE_ONGOING;
}

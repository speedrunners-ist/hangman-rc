#include "common.h"

/*** GameState implementation ***/

GameState::GameState() { active = false; }
GameState::GameState(int length, int mistakes, std::string plid) {
  wordLength = length;
  mistakesLeft = mistakes;
  playerID = plid;
  active = true;
  // word is a string with length equal to wordLength, filled with underscores
  word = std::string((size_t)length, '_');
  for (char c = 'a'; c <= 'z'; c++) {
    guessedLetters[c] = false;
  }
}

bool GameState::isActive() { return active; }

int GameState::getAvailableMistakes() { return mistakesLeft; }

void GameState::setInactive() {
  active = false;
  trials = 0;
}

char GameState::getLastGuess() { return lastGuess; }

std::string GameState::getLastWordGuess() { return lastWordGuess; }

int GameState::getWordLength() { return wordLength; }

std::string GameState::getWord() { return word; }

void GameState::setLastGuess(char guess) { lastGuess = guess; }

void GameState::setLastWordGuess(std::string guess) { lastWordGuess = guess; }

void GameState::setWord(std::string newWord) { word = newWord; }

void GameState::incorrectGuess() {
  char guess = getLastGuess();
  guessedLetters[guess] = true;
  guessesMade++;
  mistakesLeft--;
}

int GameState::correctGuess(std::string positions, int n) {
  char guess = getLastGuess();
  std::string initialWord = word;
  int readPositions = 0;
  size_t pos;
  do {
    pos = positions.find_first_of(" \n");
    std::string posStr = positions.substr(0, pos);
    const size_t posNum = (size_t)std::stoi(posStr);
    if (posNum < 1 || posNum > wordLength) {
      std::cerr << INVALID_POSITIONS_ERROR << std::endl;
      setWord(initialWord);
      return -1;
    } else if (word[posNum - 1] != '_') {
      std::cerr << ALREADY_FILLED_ERROR << std::endl;
      setWord(initialWord);
      return -1;
    }
    word[posNum - 1] = guess;
    positions = positions.substr(pos + 1);
    readPositions++;
  } while (pos != std::string::npos);

  if (n != readPositions) {
    // the answer didn't include as many positions as expected
    std::cerr << DIFF_POSITIONS_ERROR << std::endl;
    std::cerr << EXPECTED_POSITIONS(n, readPositions) << std::endl;
    setWord(initialWord);
    return -1;
  }
  std::cout << CORRECT_GUESS(getWord()) << std::endl;
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

void GameState::incrementTrials() { trials++; }
int GameState::getTrials() { return trials; }

void GameState::setPlayerID(std::string id) { playerID = id; }
std::string GameState::getPlayerID() { return playerID; }

/*** Socket functions implementation ***/

int newSocket(int type, std::string addr, std::string port, struct addrinfo *hints,
              struct addrinfo **serverInfo) {
  int socketFd = socket(AF_INET, type, 0);
  if (socketFd == -1) {
    std::cout << SOCKET_ERROR << std::endl;
    return -1;
  }
  memset(hints, 0, sizeof *hints);
  hints->ai_family = AF_INET;
  hints->ai_socktype = type;

  std::cout << "[DEBUG]: Connecting to " << addr << ":" << port << std::endl;
  int status;
  if (!addr.empty()) {
    status = getaddrinfo(addr.c_str(), port.c_str(), hints, serverInfo);
    if (status != 0) {
      std::cerr << GETADDRINFO_ERROR << std::endl;
      return -1;
    }
    return socketFd;
  }

  // in this case, it's the server creating a socket
  hints->ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, port.c_str(), hints, serverInfo);
  if (status != 0) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  if (bind(socketFd, (*serverInfo)->ai_addr, (*serverInfo)->ai_addrlen) != 0) {
    std::cout << BIND_ERROR << std::endl;
    return -1;
  }
  return socketFd;
}

int turnOnSocketTimer(int socketFd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  tv.tv_sec = SOCKET_TIMEOUT;
  if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << SOCKET_TIMER_SET_ERROR << std::endl;
    // FIXME: is this exit graceful?
    exit(EXIT_FAILURE);
  }
  return 0;
}

int turnOffSocketTimer(int socketFd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cout << SOCKET_TIMER_RESET_ERROR << std::endl;
    return -1;
  }
  return 0;
}

/*** UDP message parsing/sending implementation ***/

int disconnectUDP(struct addrinfo *res, int fd) {
  freeaddrinfo(res);
  if (close(fd) == -1) {
    std::cerr << UDP_SOCKET_CLOSE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int parseUDPMessage(std::string message, struct protocolMessage &response) {
  std::cout << "[DEBUG]: Parsing message: " << message;
  const size_t pos1 = message.find(' ');
  if (pos1 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }

  const std::string code = message.substr(0, pos1);
  const size_t pos2 = message.find_first_of(" \n", pos1 + 1);
  const char delimiter = message[pos2];
  if ((delimiter == ' ' && pos2 == std::string::npos) || (delimiter == '\n' && pos2 != message.size() - 1)) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }

  const std::string status = message.substr(pos1 + 1, pos2 - pos1 - 1);
  response = {code, pos1, status, pos2, message};
  std::cout << "[DEBUG]: Finished parsing message" << std::endl;
  return 0;
}

int sendUDPMessage(std::string message, struct addrinfo *res, int fd) {
  if (res == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;
  if (sendto(fd, message.c_str(), message.length(), 0, res->ai_addr, res->ai_addrlen) == -1) {
    std::cerr << SENDTO_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int exchangeUDPMessages(std::string message, char *response, size_t maxBytes, struct addrinfo *res, int fd) {
  if (res == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  int triesLeft = UDP_TRIES;
  do {
    if (sendUDPMessage(message, res, fd) == -1) {
      return -1;
    }

    int ret = turnOnSocketTimer(fd);
    if (ret == -1) {
      disconnectUDP(res, fd);
      exit(EXIT_FAILURE);
    }
    const ssize_t bytesReceived = recvfrom(fd, response, maxBytes, 0, res->ai_addr, &res->ai_addrlen);
    ret = turnOffSocketTimer(fd);
    if (ret == -1) {
      disconnectUDP(res, fd);
      exit(EXIT_FAILURE);
    }

    if (bytesReceived == -1) {
      if (triesLeft == 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
        break;
      }
      continue;
    }

    if (response[bytesReceived - 1] != '\n') {
      std::cerr << UDP_RESPONSE_ERROR << std::endl;
      return -1;
    }
    return 0;

  } while (--triesLeft >= 0);

  std::cerr << RECVFROM_ERROR << std::endl;
  return -1;
}

/*** Misc functions implementation ***/

int initialAvailableMistakes(int wordLength) {
  if (wordLength <= 6) {
    return 7;
  } else if (7 >= wordLength && wordLength <= 10) {
    return 8;
  }
  return 9;
}

std::string buildSplitString(std::vector<std::string> args) {
  // clang-format off
  return std::accumulate(
    ++args.begin(), args.end(), std::string(args[0]),
    [](std::string a, std::string b) { 
      return a + " " + b;
    }
  ).append("\n");
  // clang-format on
}

int readFile(std::vector<std::string> &lines, std::string filePath) {
  std::ifstream file(filePath); // TODO: do we need a flag here?
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << filePath << std::endl;
    return -1;
  }

  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }
  file.close();
  return 0;
}

int displayFile(std::string filePath, std::string dir) {
  std::ifstream file(dir + "/" + filePath);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << filePath << std::endl;
    return -1;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  return 0;
}

int validateArgsAmount(std::string input, int n) {
  const long int argCount = std::count(input.begin(), input.end(), ' ');
  // argCount will find every space in the string - ideally, one space less than the args amount
  if (argCount != n - 1 || input.back() != '\n') {
    std::cerr << DIFF_ARGS_ERROR << std::endl;
    return -1;
  }
  return 0;
}

bool validPlayerID(std::string id) {
  if (id.length() != 6) {
    std::cerr << INVALID_PLID_LEN_ERROR << std::endl;
    return false;
  }

  for (size_t i = 0; i < id.length(); i++) {
    if (!isdigit(id[i])) {
      std::cerr << INVALID_PLID_CHAR_ERROR << std::endl;
      return false;
    }
  }
  return true;
}

bool forceExit(GameState play, std::string command) { return command == "exit" && !play.isActive(); }

void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}

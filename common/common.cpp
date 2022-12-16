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
  if (mistakesLeft == 0) {
    active = 0;
  }
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

int newSocket(int type, peerInfo peer, struct addrinfo *hints, struct addrinfo **serverInfo) {
  int fd = socket(AF_INET, type, 0);
  if (fd == -1) {
    std::cout << SOCKET_ERROR << std::endl;
    return -1;
  }
  memset(hints, 0, sizeof *hints);
  hints->ai_family = AF_INET;
  hints->ai_socktype = type;

  std::cout << "[DEBUG]: Connecting to " << peer.addr << ":" << peer.port << std::endl;
  int status;
  if (!peer.addr.empty()) {
    // if we're dealing with a client, we need to resolve the address
    status = getaddrinfo(peer.addr.c_str(), peer.port.c_str(), hints, serverInfo);
    if (status != 0) {
      std::cerr << GETADDRINFO_ERROR << std::endl;
      return -1;
    }
    return fd;
  }

  // in this case, it's the server creating a socket
  hints->ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, peer.port.c_str(), hints, serverInfo);
  if (status != 0) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  if (bind(fd, (*serverInfo)->ai_addr, (*serverInfo)->ai_addrlen) != 0) {
    std::cout << BIND_ERROR << std::endl;
    return -1;
  }

  struct timeval tv;
  memset(&tv, 0, sizeof(tv));

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &tv, sizeof(tv)) < 0) {
    std::cerr << SOCKET_TIMER_SET_ERROR << std::endl;
    return -1;
  }

  return fd;
}

int disconnectSocket(struct addrinfo *res, int fd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  freeaddrinfo(res);
  if (turnOffSocketTimer(fd) == -1) {
    std::cerr << SOCKET_TIMER_SET_ERROR << std::endl;
    if (close(fd) == -1) {
      std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
    }
    return -1;
  }

  if (close(fd) == -1) {
    std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int turnOnSocketTimer(int fd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  tv.tv_sec = SOCKET_TIMEOUT;
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << SOCKET_TIMER_SET_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int turnOffSocketTimer(int fd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cout << SOCKET_TIMER_RESET_ERROR << std::endl;
    return -1;
  }
  return 0;
}

/*** UDP message parsing/sending implementation ***/

int parseUDPMessage(std::string message, protocolMessage &response) {
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

int receiveUDPMessage(char *response, size_t maxBytes, struct addrinfo *res, int fd) {
  if (res == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  const ssize_t bytesReceived = recvfrom(fd, response, maxBytes, 0, res->ai_addr, &res->ai_addrlen);
  if (bytesReceived == -1) {
    std::cerr << RECVFROM_ERROR << std::endl;
    return -1;
  } else if (response[bytesReceived - 1] != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int messageUDPHandler(int fd, struct addrinfo *res, protocolMessage &message, responseHandler handler) {
  try {
    return handler[message.first](message);
  } catch (const std::bad_function_call &e) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), res, fd);
  }
}

/*** TCP message parsing/sending implementation ***/

int parseTCPMessage(std::string request, protocolMessage &serverMessage) {
  std::string responseBegin = request;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string plid = responseBegin.substr(0, responseBegin.find_first_of(" \n"));
  serverMessage.first = command;
  serverMessage.second = plid;
  serverMessage.body = request;
  return 0;
}

int sendTCPMessage(std::string message, struct addrinfo *res, int fd) {
  if (res == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;
  if (write(fd, message.c_str(), message.length()) == -1) {
    std::cerr << TCP_SEND_MESSAGE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int sendTCPFile(std::string info, struct addrinfo *res, int fd, std::string filePath) {
  if (sendTCPMessage(info, res, fd) == -1) {
    return -1;
  }

  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }

  long fileSize = (long)std::filesystem::file_size(filePath);
  long bytesLeft = fileSize;
  long bytesSent;
  char buffer[TCP_CHUNK_SIZE];
  do {
    memset(buffer, 0, TCP_CHUNK_SIZE);
    bytesSent = (TCP_CHUNK_SIZE < bytesLeft) ? TCP_CHUNK_SIZE : bytesLeft;
    file.read(buffer, bytesSent);
    if (write(fd, buffer, (size_t)bytesSent) == -1) {
      file.close();
      return -1;
    }
    bytesLeft -= bytesSent;
  } while (bytesLeft > 0);

  file.close();
  // in the end, we must send the final delimiter, \n
  // if (sendTCPMessage("\n", fd) == -1) {
  //   return -1;
  // }
  return 0;
}

int receiveTCPMessage(std::string &message, int args, int fd) {
  ssize_t bytesReceived = 0;
  size_t bytesRead = 0;
  int readArgs = 0;
  char c;
  do {
    // FIXME: will there be a problem if the response is "ERR\n"?
    bytesReceived = read(fd, &c, 1);
    if (bytesReceived == -1) {
      std::cerr << TCP_RECV_MESSAGE_ERROR << std::endl;
      return -1;
    } else if (c == ' ' || c == '\n') {
      readArgs++;
    }
    message.push_back(c);
    bytesRead += (size_t)bytesReceived;
  } while (bytesReceived != 0 && readArgs < args);

  return (int)bytesRead;
}

int receiveTCPFile(fileInfo &info, std::string dir, int fd) {
  if (parseFileArgs(info, fd) == -1) {
    return -1;
  }

  ssize_t bytesReceived = 0;
  size_t bytesRead = 0;
  size_t bytesLeft = (size_t)info.fileSize;
  // create directory if it doesn't exist
  std::filesystem::path dirPath(dir);
  if (!std::filesystem::exists(dirPath)) {
    std::filesystem::create_directory(dirPath);
  }

  std::fstream file;
  file.open(dir + "/" + info.fileName, std::ios::out | std::ios::in | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }
  // read from socket and write to file until file size is reached, in chunks
  char buffer[TCP_CHUNK_SIZE];
  do {
    memset(buffer, 0, TCP_CHUNK_SIZE);
    bytesReceived = read(fd, buffer, (TCP_CHUNK_SIZE > bytesLeft) ? bytesLeft : TCP_CHUNK_SIZE);
    if (bytesReceived == -1) {
      std::cerr << TCP_RECV_MESSAGE_ERROR << std::endl;
      return -1;
    }
    file.write(buffer, bytesReceived);
    bytesRead += (size_t)bytesReceived;
    bytesLeft -= (size_t)bytesReceived;
  } while (bytesReceived != 0 && bytesLeft > 0);

  file.close();
  return (int)bytesRead;
}

int messageTCPHandler(int fd, struct addrinfo *res, protocolMessage &message, responseHandler handler) {
  try {
    return handler[message.first](message);
  } catch (const std::bad_function_call &e) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    return sendTCPMessage(buildSplitStringNewline({"ERR"}), res, fd);
  }
}

int parseFileArgs(fileInfo &info, int fd) {
  std::string fileArgs;
  const int ret = receiveTCPMessage(fileArgs, TCP_FILE_ARGS, fd);
  if (ret == -1) {
    std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
    return -1;
  }
  info.fileName = fileArgs.substr(0, fileArgs.find_first_of(' '));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info.fileSize = std::stoi(fileArgs.substr(0, fileArgs.find_first_of(' ')));
  fileArgs.erase(0, fileArgs.find_first_of(' '));
  info.delimiter = fileArgs[0];
  if (info.delimiter == ' ') {
    return 0;
  }
  std::cerr << INVALID_FILE_ARGS << std::endl;
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
  );
  // clang-format on
}

std::string buildSplitStringNewline(std::vector<std::string> args) {
  return buildSplitString(args).append("\n");
}

int readFile(std::vector<std::string> &lines, std::string filePath) {
  std::ifstream file(filePath);

  // if file doesn't exist
  if (file.fail()) {
    std::cerr << FILE_DOES_NOT_EXIST << " File: " << filePath << std::endl;
    return -2;
  }

  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << " File: " << filePath << std::endl;
    return -1;
  }

  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }
  file.close();
  return 0;
}

int displayFile(std::string filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << " File: " << filePath << std::endl;
    return -1;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  return 0;
}

bool validArgsAmount(std::string input, int n) {
  return std::count(input.begin(), input.end(), ' ') == n - 1;
}

bool validResponse(std::string body, std::vector<int> &args, int expectedArgs) {
  std::stringstream ss(body);
  std::vector<std::string> tokens;
  std::string token;
  int readArgs = 0;
  int arg;
  while (ss >> token && readArgs++ < expectedArgs) {
    try {
      arg = std::stoi(token);
      args.push_back(arg);
      tokens.push_back(token);
    } catch (std::invalid_argument &e) {
      std::cout << "[ERR]: Invalid argument: " << token << std::endl;
      return false;
    }
  }
  return true; // TODO: check if this is right?
}

bool forceExit(GameState state, std::string command) { return command == "exit" && !state.isActive(); }

void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}

void toLower(std::string &str) { std::transform(str.begin(), str.end(), str.begin(), ::tolower); }

bool validPlayerID(std::string plid) { return plid.length() == 6 && isNumber(plid); }

bool isNumber(std::string trial) { return std::all_of(trial.begin(), trial.end(), ::isdigit); }

bool hasWordFormat(std::string word) { return std::all_of(word.begin(), word.end(), ::isalpha); }

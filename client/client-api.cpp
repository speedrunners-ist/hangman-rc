#include "client-api.h"

socketInfo socketTCP, socketUDP;

/*** ClientAPI methods implementation ***/

GameState state;
std::string plid;

void createGame(std::vector<int> args, std::string playerID) {
  state = GameState(args[0], args[1], playerID);
}

int playCorrectGuess(std::string positions, int n) {
  if (n == 0) {
    return -1;
  }
  int ret = state.correctGuess(positions, n);
  if (ret == 0) {
    state.incrementTrials();
  }
  return ret;
}

void playIncorrectGuess() {
  incrementTrials();
  state.incorrectGuess();
}

void playCorrectFinalGuess() { state.correctFinalGuess(); }

void playCorrectFinalWordGuess() { state.correctFinalWordGuess(); }

int getAvailableMistakes() { return state.getAvailableMistakes(); }

std::string getWord() { return state.getWord(); }

void setLastGuess(char guess) { state.setLastGuess(guess); }

void setLastWordGuess(std::string guess) { state.setLastWordGuess(guess); }

int getWordLength() { return state.getWordLength(); }

void setPlayerID(std::string id) { plid = id; }

std::string getPlayerID() { return plid; }

void resetGame() { state.setInactive(); }

void incrementTrials() { state.incrementTrials(); }

int getTrials() { return state.getTrials(); }

bool forceExitClient(std::string command) { return forceExit(state, command); }

std::vector<std::string> getKeys(commandHandler map) {
  std::vector<std::string> keys;
  for (auto const &pair : map) {
    keys.push_back(pair.first);
  }
  return keys;
}

void printHelpMenu() {
  std::cout << "Welcome to a brand new Hangman game!" << std::endl;
  std::cout << "To play, you can use the following commands:" << std::endl;
  std::cout << "  - start/sg <player_id>: starts a new game. The Player ID must be composed of 6 digits."
            << std::endl;
  std::cout << "  - play/pl <letter>: works as a letter guess." << std::endl;
  std::cout << "  - guess/gw <word>: works as a word guess." << std::endl;
  std::cout << "  - scoreboard/sb: shows the scoreboard, with the Top 10 scores stored by the server."
            << std::endl;
  std::cout << "  - hint/h: shows a hint for the current word." << std::endl;
  std::cout << "  - state/st: shows the current state of the game (or the state of your last finished game)."
            << std::endl;
  std::cout << "  - rev: if in development, shows the current word. Otherwise, does nothing." << std::endl;
  std::cout << "  - quit: quits the game." << std::endl;
  std::cout << "  - exit: quits the game and exits the program." << std::endl;
  std::cout << "  Good luck!" << std::endl;
}

void displayCurrentInformation() {
  std::cout << "----------------------------------------" << std::endl;
  if (state.isActive()) {
    std::cout << "Current game information:" << std::endl;
    std::cout << "  - Word (as currently guessed): " << getWord() << std::endl;
    std::cout << "  - Available mistakes: " << getAvailableMistakes() << std::endl;
    return;
  }
  std::cout << "No game currently active." << std::endl;
}

// Creation and retrieval of client socket information (both UDP and TCP)

int createSocket(__socket_type type, peerInfo peer) {
  socketInfo socket;
  socket.fd = newSocket(type, peer, &socket.hints, &socket.res);
  if (socket.fd == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (type == SOCK_STREAM) {
    socket.isConnected = true;
    if (connect(socket.fd, socket.res->ai_addr, socket.res->ai_addrlen) == -1) {
      std::cerr << TCP_SERVER_ERROR << std::endl;
      disconnect(socket);
      return -1;
    }
  }

  if (turnOnSocketTimer(socket.fd) == -1) {
    disconnect(socket);
    return -1;
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  memset(&socket.act, 0, sizeof(socket.act));
  socket.act.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &socket.act, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    disconnect(socket);
    return -1;
  }
  socket.type = type;
  if (type == SOCK_STREAM) {
    socket.isConnected = true;
    socketTCP = socket;
    return socketTCP.fd;
  }
  socketUDP = socket;
  return socket.fd;
}

int disconnect(socketInfo socket) {
  if (socket.type == SOCK_STREAM) {
    socketTCP.isConnected = false;
  } else {
    sendUDPMessage(buildSplitStringNewline({"QUT", getPlayerID()}), socket.res, socket.fd);
  }
  return disconnectSocket(socket.res, socket.fd);
}

socketInfo getSocket(__socket_type type) { return type == SOCK_STREAM ? socketTCP : socketUDP; }
struct addrinfo *getServerInfoUDP() { return socketUDP.res; }
struct addrinfo *getServerInfoTCP() { return socketTCP.res; }
int getSocketFdUDP() { return socketUDP.fd; }
int getSocketFdTCP() { return socketTCP.fd; }
std::string getExpectedMessageUDP() { return socketUDP.expectedMessage; }
std::string getExpectedMessageTCP() { return socketTCP.expectedMessage; }
void setExpectedMessageUDP(std::string message) { socketUDP.expectedMessage = message; }
void setExpectedMessageTCP(std::string message) { socketTCP.expectedMessage = message; }

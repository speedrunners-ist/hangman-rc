#include "client-protocol.h"

struct addrinfo *serverInfoUDP;
struct addrinfo hintsUDP;
int socketFdUDP;
struct sigaction actUDP;

// clang-format off
responseHandler handleUDPServerMessage = {
  {"RSG", handleRSG},
  {"RLG", handleRLG},
  {"RWG", handleRWG},
  {"RQT", handleRQT},
  {"RRV", handleRRV}
};
// clang-format on

int createSocketUDP(struct peerInfo peer) {
  socketFdUDP = newSocket(SOCK_DGRAM, peer, &hintsUDP, &serverInfoUDP);
  if (socketFdUDP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (turnOnSocketTimer(socketFdUDP) == -1) {
    disconnectUDP();
    return -1;
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  memset(&actUDP, 0, sizeof(actUDP));
  actUDP.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actUDP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    disconnectUDP();
    return -1;
  }
  return socketFdUDP;
}

int disconnectUDP() { return disconnectSocket(serverInfoUDP, socketFdUDP); }

int generalUDPHandler(std::string message, size_t maxBytes) {
  char responseMessage[maxBytes + 1];
  memset(responseMessage, 0, maxBytes + 1);
  struct protocolMessage response;
  int ret = exchangeUDPMessages(message, responseMessage, maxBytes, serverInfoUDP, socketFdUDP);
  ret = parseUDPMessage(responseMessage, response);
  if (ret == -1) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  return handleUDPServerMessage[response.first](response);
}

// Server response handlers
int handleRSG(struct protocolMessage response) {
  if (response.second == "OK") {
    std::string body = response.body.substr(response.secondPos + 1);
    body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
    std::vector<int> args;
    if (!validResponse(body, args, 2)) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    createGame(args, getPlayerID());
    const int availableMistakes = getAvailableMistakes();
    const std::string word = getWord();
    std::cout << RSG_OK(availableMistakes, word) << std::endl;
    return 0;
  } else if (response.second == "NOK") {
    std::cout << RSG_NOK << std::endl;
    return 0;
  } else if (response.second == "ERR") {
    std::cout << RSG_ERR << std::endl;
    return 0;
  }

  return -1;
}

int handleRLG(struct protocolMessage response) {
  if (response.second == "OK") {
    std::string body = response.body.substr(response.secondPos + 1);
    body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
    std::vector<int> args;
    if (!validResponse(body, args, 2)) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    const int n = args[1];
    for (int i = 0; i < 2; i++) {
      // erase trial and n from body
      body = body.substr(body.find(' ') + 1);
    }
    return playCorrectGuess(body, n);
  } else if (response.second == "WIN") {
    playCorrectFinalGuess();
    resetGame();
    std::cout << RLG_WIN(getWord()) << std::endl;
    return 0;
  } else if (response.second == "DUP") {
    std::cout << RLG_DUP << std::endl;
    return 0;
  } else if (response.second == "NOK") {
    playIncorrectGuess();
    std::cout << RLG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  } else if (response.second == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RLG_OVR << std::endl;
    return 0;
  } else if (response.second == "INV") {
    std::cout << RLG_INV << std::endl;
    return 0;
  } else if (response.second == "ERR") {
    std::cout << RLG_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRWG(struct protocolMessage response) {
  std::string body = response.body.substr(response.secondPos + 1);
  body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
  std::vector<int> args;
  if (!validResponse(body, args, 1)) {
    std::cerr << RWG_ERROR << std::endl;
    return -1;
  }
  if (response.second == "WIN") {
    playCorrectFinalWordGuess();
    resetGame();
    std::cout << RWG_WIN(getWord()) << std::endl;
    return 0;
  } else if (response.second == "NOK") {
    playIncorrectGuess();
    std::cout << RWG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  } else if (response.second == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RWG_OVR << std::endl;
    return 0;
  } else if (response.second == "INV") {
    std::cout << RWG_INV << std::endl;
    return 0;
  } else if (response.second == "ERR") {
    std::cout << RWG_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRQT(struct protocolMessage response) {
  if (response.second == "OK") {
    std::cout << RQT_OK << std::endl;
    return 0;
  } else if (response.second == "ERR") {
    if (response.first == "quit") {
      std::cout << RQT_ERR << std::endl;
    }
    return 0;
  }
  return -1;
}

int handleRRV(struct protocolMessage response) {
  std::cout << "[REV]: Word is " << response.second << std::endl;
  return 0;
}

// handlers: player requests
int sendSNG(struct messageInfo info) {
  if (!validArgsAmount(info.input, START_ARGS)) {
    return -1;
  }
  const size_t pos1 = info.input.find(' ');
  std::string plid = info.input.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());
  if (!validPlayerID(plid)) {
    return -1;
  }
  setPlayerID(plid);
  const std::string message = buildSplitStringNewline({"SNG", plid});
  return generalUDPHandler(message, RSG_BYTES);
}

int sendPLG(struct messageInfo info) {
  if (!validArgsAmount(info.input, PLAY_ARGS)) {
    return -1;
  } else if (getPlayerID() == "") {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }

  const size_t pos1 = info.input.find(' ');
  std::string letter = info.input.substr(pos1 + 1);
  letter.erase(std::remove(letter.begin(), letter.end(), '\n'), letter.end());
  if (letter.length() != 1 || !std::isalpha(letter[0])) {
    std::cerr << EXPECTED_LETTER_ERROR << std::endl;
    return -1;
  }
  const std::string message =
      buildSplitStringNewline({"PLG", getPlayerID(), letter, std::to_string(getTrials() + 1)});
  setLastGuess(letter[0]);
  return generalUDPHandler(message, RLG_BYTES);
}

int sendPWG(struct messageInfo info) {
  if (!validArgsAmount(info.input, GUESS_ARGS)) {
    return -1;
  } else if (getPlayerID() == "") {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }
  const size_t pos1 = info.input.find(' ');
  std::string guess = info.input.substr(pos1 + 1);
  guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());
  if (guess.length() != getWordLength()) {
    std::cerr << EXPECTED_WORD_DIF_LEN_ERROR(getWordLength()) << std::endl;
    return -1;
  }
  const std::string message =
      buildSplitStringNewline({"PWG", getPlayerID(), guess, std::to_string(getTrials() + 1)});
  setLastWordGuess(guess);
  return generalUDPHandler(message, RWG_BYTES);
}

int sendQUT(struct messageInfo info) {
  if (!validArgsAmount(info.input, QUIT_ARGS)) {
    return -1;
  } else if (getPlayerID() == "") {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }
  const std::string command = info.input.substr(0, info.input.find('\n'));
  const std::string message = buildSplitStringNewline({"QUT", getPlayerID()});
  if (command == "quit") {
    return generalUDPHandler(message, RQT_BYTES);
  }
  return generalUDPHandler(message, RQT_BYTES) == 0 ? EXIT_HANGMAN : -1;
}

int sendREV(struct messageInfo info) {
  if (!validArgsAmount(info.input, REVEAL_ARGS)) {
    return -1;
  } else if (getPlayerID() == "") {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }
  const std::string message = buildSplitStringNewline({"REV", getPlayerID()});
  return generalUDPHandler(message, RRV_BYTES);
}

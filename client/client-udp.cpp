#include "client-protocol.h"

struct addrinfo *serverInfoUDP;
int socketFdUDP;

// TODO: change this to read a specific amount of bytes from the socket for each specific command

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
  socketFdUDP = newSocket(SOCK_DGRAM, peer.addr, peer.port, &serverInfoUDP);
  return socketFdUDP;
}

int disconnectUDP() {
  freeaddrinfo(serverInfoUDP);
  if (close(socketFdUDP) == -1) {
    std::cerr << UDP_SOCKET_CLOSE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int exchangeUDPMessage(std::string message, char *response, size_t maxExpectedBytes) {
  if (serverInfoUDP == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  int triesLeft = UDP_TRIES;
  do {
    if (sendto(socketFdUDP, message.c_str(), message.length(), 0, serverInfoUDP->ai_addr,
               serverInfoUDP->ai_addrlen) == -1) {
      std::cerr << SENDTO_ERROR << std::endl;
      return -1;
    }

    socklen_t addrLen = sizeof(serverInfoUDP->ai_addr);
    turnOnSocketTimer(socketFdUDP);
    const ssize_t bytesReceived =
        recvfrom(socketFdUDP, response, maxExpectedBytes, 0, serverInfoUDP->ai_addr, &addrLen);
    turnOffSocketTimer(socketFdUDP);
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
    response[bytesReceived - 1] = '\0';
    return 0;

  } while (--triesLeft >= 0);

  std::cerr << RECVFROM_ERROR << std::endl;
  return -1;
}

int parseUDPResponse(char *response) {
  const std::string responseStr(response);
  const size_t pos1 = responseStr.find(' ');
  if (pos1 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  const bool lookingForEndLine = (code == "RQT" || code == "RRV");
  const char lookupStatusChar = lookingForEndLine ? '\n' : ' ';
  const size_t pos2 = responseStr.find(lookupStatusChar, pos1 + 1);
  if (lookingForEndLine && pos2 != std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  } else if (!lookingForEndLine && pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  const struct protocolMessage serverResponse = {code, pos1, status, pos2, responseStr};
  return handleUDPServerMessage[code](serverResponse);
}

// UDP handlers
int generalUDPHandler(std::string message, size_t maxExpectedBytes) {
  char responseUDP[maxExpectedBytes];
  memset(responseUDP, 0, maxExpectedBytes);
  const int ret = exchangeUDPMessage(message, responseUDP, maxExpectedBytes);
  if (ret == -1) {
    return -1;
  }
  return parseUDPResponse(responseUDP);
}

// handlers: server responses
// TODO: can't forget to check if the response is valid, ending with \n
int handleRSG(struct protocolMessage response) {
  if (response.status == "OK") {
    const size_t pos_n_letters = response.body.find(' ', response.statusPos + 1);
    const size_t pos_n_max_errors = response.body.find('\n', pos_n_letters + 1);
    if (pos_n_letters == std::string::npos || pos_n_max_errors != std::string::npos) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    // TODO: check if n_letters and n_max_errors are valid
    const int n_letters = std::stoi(
        response.body.substr(response.statusPos + 1, pos_n_letters - response.statusPos - 1));
    const int n_max_errors =
        std::stoi(response.body.substr(pos_n_letters + 1, pos_n_max_errors - pos_n_letters - 1));
    createGame(n_letters, n_max_errors);
    const int availableMistakes = getAvailableMistakes();
    const std::string word = getWord();
    std::cout << RSG_OK(availableMistakes, word) << std::endl;
    return 0;
  } else if (response.status == "NOK") {
    std::cout << RSG_NOK << std::endl;
    return 0;
  }

  return -1;
}

int handleRLG(struct protocolMessage response) {
  const size_t pos_trial = response.body.find_first_of(' ', response.statusPos + 1);
  if (response.status == "OK") {
    const size_t pos_n = response.body.find(' ', pos_trial + 1);
    const size_t pos_correct_positions = response.body.find('\n', pos_n + 1);
    if (pos_n == std::string::npos || pos_correct_positions != std::string::npos) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    const int n = std::stoi(response.body.substr(pos_trial + 1, pos_n - pos_trial - 1));
    if (playCorrectGuess(response.body.substr(pos_n + 1), n) == 0) {
      return 0;
    }
  } else if (response.status == "WIN") {
    playCorrectFinalGuess();
    resetGame();
    std::cout << RLG_WIN(getWord()) << std::endl;
    return 0;
  } else if (response.status == "DUP") {
    std::cout << RLG_DUP << std::endl;
    return 0;
  } else if (response.status == "NOK") {
    playIncorrectGuess();
    std::cout << RLG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  } else if (response.status == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RLG_OVR << std::endl;
    return 0;
  } else if (response.status == "INV") {
    std::cout << RLG_INV << std::endl;
    return 0;
  } else if (response.status == "ERR") {
    std::cout << RLG_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRWG(struct protocolMessage response) {
  const size_t pos_trials = response.body.find('\n', response.statusPos + 1);
  if (pos_trials != std::string::npos) {
    std::cerr << RWG_ERROR << std::endl;
    return -1;
  }
  if (response.status == "WIN") {
    playCorrectFinalWordGuess();
    resetGame();
    std::cout << RWG_WIN(getWord()) << std::endl;
    return 0;
  } else if (response.status == "NOK") {
    playIncorrectGuess();
    std::cout << RWG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  } else if (response.status == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RWG_OVR << std::endl;
    return 0;
  } else if (response.status == "INV") {
    std::cout << RWG_INV << std::endl;
    return 0;
  } else if (response.status == "ERR") {
    std::cout << RWG_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRQT(struct protocolMessage response) {
  if (response.status == "OK") {
    std::cout << RQT_OK << std::endl;
    return 0;
  } else if (response.status == "ERR") {
    if (response.code == "quit") {
      std::cout << RQT_ERR << std::endl;
    }
    return 0;
  }
  return -1;
}

int handleRRV(struct protocolMessage response) {
  // TODO: change this for the final version of the project
  // to-be-guessed word is the second argument - here, the status
  std::cout << "[DEBUG/RRV]: Word is " << response.status << std::endl;
  return 0;
}

// handlers: player requests
int sendSNG(struct messageInfo info) {
  if (validateArgsAmount(info.input, START_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = info.input.find(' ');
  std::string plid = info.input.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());
  if (validatePlayerID(plid) == 0) {
    setPlayerID(plid);
    const std::string message = buildSplitString({"SNG", plid});
    return generalUDPHandler(message, RSG_BYTES);
  }
  return -1;
}

int sendPLG(struct messageInfo info) {
  if (validateArgsAmount(info.input, PLAY_ARGS) == -1) {
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
      buildSplitString({"PLG", getPlayerID(), letter, std::to_string(getTrials() + 1)});
  setLastGuess(letter[0]);
  return generalUDPHandler(message, RLG_BYTES);
}

int sendPWG(struct messageInfo info) {
  if (validateArgsAmount(info.input, GUESS_ARGS) == -1) {
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
      buildSplitString({"PWG", getPlayerID(), guess, std::to_string(getTrials() + 1)});
  setLastWordGuess(guess);
  return generalUDPHandler(message, RWG_BYTES);
}

int sendQUT(struct messageInfo info) {
  // TODO: can't forget to close all open TCP connections - what do we have to do here, exactly?
  if (validateArgsAmount(info.input, QUIT_ARGS) == -1) {
    return -1;
  }
  const std::string command = info.input.substr(0, info.input.find('\n'));
  const std::string message = buildSplitString({"QUT", getPlayerID()});
  if (command == "quit") {
    return generalUDPHandler(message, RQT_BYTES);
  }
  return generalUDPHandler(message, RQT_BYTES) == 0 ? EXIT_HANGMAN : -1;
}

int sendREV(struct messageInfo info) {
  if (validateArgsAmount(info.input, REVEAL_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"REV", getPlayerID()});
  return generalUDPHandler(message, RRV_BYTES);
}

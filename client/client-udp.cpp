#include "client-protocol.h"

struct addrinfo *serverInfoUDP;
struct addrinfo hintsUDP;
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
  socketFdUDP = newSocket(SOCK_DGRAM, peer.addr, peer.port, &hintsUDP, &serverInfoUDP);
  return socketFdUDP;
}

int disconnectPlayer() { return disconnectUDP(serverInfoUDP, socketFdUDP); }

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
  return handleUDPServerMessage[response.code](response);
}

// Server response handlers
int handleRSG(struct protocolMessage response) {
  if (response.status == "OK") {
    const size_t pos_n_letters = response.body.find(' ', response.statusPos + 1);
    const size_t pos_n_max_errors = response.body.find('\n', pos_n_letters + 1);
    if (pos_n_letters == std::string::npos || pos_n_max_errors != std::string::npos) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    // TODO: check if n_letters and n_max_errors are valid
    const int n_letters =
        std::stoi(response.body.substr(response.statusPos + 1, pos_n_letters - response.statusPos - 1));
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
  std::cout << "[REV]: Word is " << response.status << std::endl;
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

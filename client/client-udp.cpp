#include "client-protocol.h"

static struct addrinfo *serverInfo;
static int socketFd;
static char responseUDP[UDP_RECV_SIZE];

// clang-format off
static responseHandler handleUDPServerMessage = {
  {"RSG", handleRSG},
  {"RLG", handleRLG},
  {"RWG", handleRWG},
  {"RQT", handleRQT},
  {"RRV", handleRRV}
};
// clang-format on

// TODO: in order for the program to exit gracefully, we always need to close any open sockets!!

// Creates a new socket and connects to the server
int newSocket(int type, std::string addr, std::string port) {
  socketFd = socket(AF_INET, type, 0);
  if (socketFd == -1) {
    // FIXME: should we really exit here?
    std::cout << "[ERR]: Failed to create socket. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = type;

  const int status = getaddrinfo(addr.c_str(), port.c_str(), &hints, &serverInfo);
  if (status != 0) {
    std::cout << "[ERR]: Failed to get address info. Exiting." << std::endl;
    return -1;
  }
  return 0;
}

int exchangeUDPMessage(std::string message, char *response) {
  if (serverInfo == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;

  std::cout << message.length() << " bytes" << std::endl;

  int triesLeft = UDP_TRIES;
  do {
    // note: we don't send the null terminator, hence the -1
    if (sendto(socketFd, message.c_str(), message.length(), 0, serverInfo->ai_addr,
               serverInfo->ai_addrlen) == -1) {
      std::cerr << SENDTO_ERROR << std::endl;
      return -1;
    }

    socklen_t addrLen = sizeof(serverInfo->ai_addr);
    const ssize_t bytesReceived =
        recvfrom(socketFd, response, UDP_RECV_SIZE, 0, serverInfo->ai_addr, &addrLen);

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
  const char lookupStatusChar = (code == "RQT" || code == "RRV") ? '\n' : ' ';
  const size_t pos2 = responseStr.find(lookupStatusChar, pos1 + 1);
  if (pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  const struct serverResponse serverResponse = {code, pos1, status, pos2, responseStr};
  return handleUDPServerMessage[code](serverResponse);
}

// UDP handlers
int generalUDPHandler(std::string message) {
  memset(responseUDP, 0, UDP_RECV_SIZE);
  const int ret = exchangeUDPMessage(message, responseUDP);
  if (ret == -1) {
    return -1;
  }
  return parseUDPResponse(responseUDP);
}

// handlers: server responses
// TODO: can't forget to check if the response is valid, ending with \n
int handleRSG(struct serverResponse response) {
  if (response.status == "OK") {
    const size_t pos_n_letters = response.body.find(' ', response.statusPos + 1);
    const size_t pos_n_max_errors = response.body.find('\n', pos_n_letters + 1);
    if (pos_n_letters == std::string::npos || pos_n_max_errors == std::string::npos) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    // TODO: check if n_letters and n_max_errors are valid
    const int n_letters = std::stoi(response.body.substr(response.statusPos + 1, pos_n_letters));
    const int n_max_errors = std::stoi(response.body.substr(pos_n_letters + 1, pos_n_max_errors));
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

int handleRLG(struct serverResponse response) {
  const size_t pos_trial = response.body.find(' ', response.statusPos + 1);
  if (pos_trial == std::string::npos) {
    std::cerr << RLG_ERROR << std::endl;
    return -1;
  }
  if (response.status == "OK") {
    const size_t pos_n = response.body.find(' ', pos_trial + 1);
    const size_t pos_correct_positions = response.body.find('\n', pos_n + 1);
    if (pos_n == std::string::npos || pos_correct_positions == std::string::npos) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    const int n = std::stoi(response.body.substr(pos_trial + 1, pos_n));
    if (n < 3 || n > 30) {
      std::cerr << RLG_INVALID_WORD_LEN << std::endl;
      return -1;
    }
    if (playCorrectGuess(response.body.substr(pos_n + 1), n) == 0) {
      return 0;
    }
  } else if (response.status == "WIN") {
    playCorrectFinalGuess();
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
    // the server itself ends the game on its end, so we should add a mechanism on our end
    // to end the game as well ig
    playIncorrectGuess();
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

int handleRWG(struct serverResponse response) {
  const size_t pos_trials = response.body.find('\n', response.statusPos + 1);
  if (pos_trials == std::string::npos) {
    std::cerr << RWG_ERROR << std::endl;
    return -1;
  }
  if (response.status == "WIN") {
    playCorrectFinalGuess();
    std::cout << RWG_WIN(getWord()) << std::endl;
    return 0;
  } else if (response.status == "NOK") {
    playIncorrectGuess();
    std::cout << RWG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  } else if (response.status == "OVR") {
    // the server itself ends the game on its end, so we should add a mechanism on our end
    // to end the game as well ig
    playIncorrectGuess();
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

int handleRQT(struct serverResponse response) {
  if (response.status == "OK") {
    std::cout << RQT_OK << std::endl;
    return 0;
  } else if (response.status == "ERR") {
    std::cout << RQT_ERR << std::endl;
    return 0;
  }
  return -1;
}

// TODO: implement debug command below
int handleRRV(struct serverResponse response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

// handlers: player requests
int handleSNG(std::string input) {
  if (validateArgsAmount(input, SNG_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == 0) {
    const std::string message = buildPlayerMessage({"SNG", plid});
    return generalUDPHandler(message);
  }
  return -1;
}

int handlePLG(std::string input) {
  if (validateArgsAmount(input, PLG_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const size_t pos2 = input.find(' ');
  const size_t pos3 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1, pos2 - pos1 - 1);
  const std::string letter = input.substr(pos2 + 1, pos3 - pos2 - 1);
  const std::string trial = input.substr(pos3 + 1);
  if (letter.length() != 1 || !std::isalpha(letter[0])) {
    std::cerr << EXPECTED_LETTER_ERROR << std::endl;
    return -1;
  } else if (validatePlayerID(plid) == -1) {
    return -1;
  } // FIXME: should we validate the trial?
  const std::string message = buildPlayerMessage({"PLG", plid, letter, trial});
  setLastGuess(letter[0]);
  return generalUDPHandler(message);
}

int handlePWG(std::string input) {
  if (validateArgsAmount(input, PWG_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const size_t pos2 = input.find(' ');
  const size_t pos3 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1, pos2 - pos1 - 1);
  const std::string guess = input.substr(pos2 + 1, pos3 - pos2 - 1);
  const std::string trial = input.substr(pos3 + 1);
  if (guess.length() != getWordLength()) {
    std::cerr << EXPECTED_WORD_DIF_LEN_ERROR(getWordLength()) << std::endl;
    return -1;
  } else if (validatePlayerID(plid) == -1) {
    return -1;
  } // FIXME: should we validate the trial?
  const std::string message = buildPlayerMessage({"PWG", plid, guess, trial});
  return generalUDPHandler(message);
}

int handleQUT(std::string input) {
  // TODO: can't forget to close all open TCP connections
  if (validateArgsAmount(input, QUT_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string command = input.substr(0, pos1);
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildPlayerMessage({"QUT", plid});
  if (command == "quit") {
    return generalUDPHandler(message);
  }
  return generalUDPHandler(message) == 0 ? EXIT_HANGMAN : -1;
}

int handleREV(std::string input) {
  if (validateArgsAmount(input, REV_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildPlayerMessage({"REV", plid});
  return generalUDPHandler(message);
}

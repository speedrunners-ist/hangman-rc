#include "client-protocol.h"

static struct addrinfo *serverInfo;
static int socketFd;
static char responseUDP[UDP_RECV_SIZE];
static int trials = 0;

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

  std::cout << message.length() << std::endl;

  int triesLeft = UDP_TRIES;
  do {
    // note: we don't send the null terminator, hence the -1
    if (sendto(socketFd, message.c_str(), message.length(), 0, serverInfo->ai_addr,
               serverInfo->ai_addrlen) == -1) {
      std::cerr << SENDTO_ERROR << std::endl;
      return -1;
    }

    socklen_t addrLen = sizeof(serverInfo->ai_addr);
    ssize_t bytesReceived =
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

// make sure we test formatting for every parameter in every response
int parseUDPResponse(char *response) {
  // TODO: abstract this into separate functions
  std::string responseStr(response);
  size_t pos1 = responseStr.find(' ');
  size_t pos2 = responseStr.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  if (code == "RSG") {
    if (status == "OK") {
      size_t pos3 = responseStr.find(' ', pos2 + 1);
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      if (pos3 == std::string::npos || pos4 == std::string::npos) {
        std::cerr << RSG_ERROR << std::endl;
        return -1;
      }

      // TODO: check if the word length is valid?
      play = Play(std::stoi(responseStr.substr(pos2 + 1, pos3 - pos2 - 1)),
                  std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1)));
      std::cout << RSG_OK(play.getAvailableMistakes(), play.getWord()) << std::endl;
      return 0;
    } else if (status == "NOK") {
      std::cout << RSG_NOK << std::endl;
      return 0;
    }
    // unknown status
    std::cerr << RSG_ERROR << std::endl;
  } else if (code == "RLG") {
    size_t pos3 = responseStr.find(' ', pos2 + 1);
    if (pos3 == std::string::npos) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    if (status == "OK") {
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      size_t pos5 = responseStr.find(' ', pos4 + 1);
      if (pos4 == std::string::npos || pos5 == std::string::npos) {
        std::cerr << RLG_ERROR << std::endl;
        return -1;
      }
      int n = std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1));
      if (n < 3 || n > 30) {
        std::cerr << RLG_INVALID_WORD_LEN << std::endl;
        return -1;
      }
      if (play.correctGuess(responseStr.substr(pos4 + 1), n) == 0) {
        trials++;
        return 0;
      }
    } else if (status == "WIN") {
      play.correctFinalGuess();
      std::cout << RLG_WIN(play.getWord()) << std::endl;
      trials++;
      return 0;
    } else if (status == "DUP") {
      std::cout << RLG_DUP << std::endl;
      return 0;
    } else if (status == "NOK") {
      play.incorrectGuess();
      std::cout << RLG_NOK(play.getAvailableMistakes()) << std::endl;
      trials++;
      return 0;
    } else if (status == "OVR") {
      play.incorrectGuess();
      std::cout << RLG_OVR << std::endl;
      trials++;
      return 0;
    } else if (status == "INV") {
      std::cout << RLG_INV << std::endl;
    } else if (status == "ERR") {
      std::cout << RLG_ERR << std::endl;
    }
  } else if (code == "RWG") {
    // TODO: don't forget to increment trials here
  } // TODO: implement the rest
  return -1;
}

// UDP handlers

int generalUDPHandler(std::string message) {
  memset(responseUDP, 0, UDP_RECV_SIZE);
  int ret = exchangeUDPMessage(message, responseUDP);
  if (ret == -1) {
    return -1;
  }
  return parseUDPResponse(responseUDP);
}

int handleStart(std::string message, std::string input) {
  if (validateTwoArgsCommand(input) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  std::string plid = input.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  if (plid.length() != 6) {
    std::cerr << INVALID_PLID_LEN_ERROR << std::endl;
    return -1;
  }

  for (size_t i = 0; i < plid.length(); i++) {
    if (!isdigit(plid[i])) {
      std::cerr << INVALID_PLID_CHAR_ERROR << std::endl;
      return -1;
    }
  }

  playerID = plid;
  message = "SNG " + plid + "\n";
  // DEBUG: checking if last character is a newline
  if (message.back() != '\n') {
    std::cerr << "[ERR]: Last character is not a newline. Not sending a message." << std::endl;
    return -1;
  }
  return generalUDPHandler(message);
}

int handlePlay(std::string message, std::string input) {
  if (validateTwoArgsCommand(input) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  std::string letter = input.substr(pos1 + 1);
  letter.erase(std::remove(letter.begin(), letter.end(), '\n'), letter.end());
  if (letter.length() != 1 || !std::isalpha(letter[0])) {
    std::cerr << EXPECTED_LETTER_ERROR << std::endl;
    return -1;
  }
  message = "PLG " + playerID + " " + letter + " " + std::to_string(trials + 1) + "\n";
  play.setLastGuess(letter[0]);
  return generalUDPHandler(message);
}

int handleGuess(std::string message, std::string input) {
  if (validateTwoArgsCommand(input) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  std::string guess = input.substr(pos1 + 1);
  guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());
  if (guess.length() != play.getWordLength()) {
    std::cerr << EXPECTED_WORD_DIF_LEN_ERROR << play.getWordLength() << std::endl;
    return -1;
  }
  message = "PWG " + playerID + " " + guess + " " + std::to_string(trials + 1) + "\n";
  return generalUDPHandler(message);
}

int handleQuit(std::string message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  message = "QUT " + playerID + "\n";
  return generalUDPHandler(message);
}

int handleExit(std::string message, std::string input) { return handleQuit(message, input); }

int handleDebug(std::string message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  message = "REV " + playerID + "\n";
  return generalUDPHandler(message);
}

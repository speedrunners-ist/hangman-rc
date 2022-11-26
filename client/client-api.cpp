#include "client-api.h"

int newSocket(int type, std::string addr, std::string port) {
  const int socketFd = socket(AF_INET, type, 0);
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
  return socketFd;
}

int validateSingleArgCommand(std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cerr << TOO_MANY_ARGS_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int validateTwoArgsCommand(std::string input) {
  const size_t pos1 = input.find(' ');
  const size_t pos2 = input.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 != std::string::npos) {
    std::cerr << TOO_MANY_ARGS_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int handleStart(std::string *message, std::string input) {
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
  *message = "RSG " + plid + "\n";
  // DEBUG: checking if last character is a newline
  if (message->back() != '\n') {
    std::cerr << "[ERR]: Last character is not a newline. Not sending a message." << std::endl;
    return -1;
  }
  return 0;
}

int handlePlay(std::string *message, std::string input) {
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
  *message = "PLG " + playerID + " " + letter + " " + std::to_string(trials + 1) + "\n";
  play.setLastGuess(letter[0]);
  return 0;
}

int handleGuess(std::string *message, std::string input) {
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
  *message = "PWG " + playerID + " " + guess + " " + std::to_string(trials + 1) + "\n";
  return 0;
}

int handleScoreboard(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "GSB\n";
  return 0;
}

int handleHint(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "GHL " + playerID + "\n";
  return 0;
}

int handleState(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "STA " + playerID + "\n";
  return 0;
}

int handleQuit(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "QUT " + playerID + "\n";
  return 0;
}

int handleExit(std::string *message, std::string input) { return handleQuit(message, input); }

int handleDebug(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "REV " + playerID + "\n";
  return 0;
}

void exitGracefully(std::string errorMessage) {
  std::cerr << errorMessage << std::endl;
  close(fd);
  freeaddrinfo(serverInfo);
  exit(EXIT_SUCCESS);
}

void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}
#include "client-protocol.h"

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

// TODO: handler functions' beginning is very similar, abstract it
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

int main(int argc, char *argv[]) {
  // TODO: maybe consider a parsing function?
  // command format: ./player [-n GSIP] [-p GSport]
  // GSIP: IP address of the game server
  // GSport: port number of the game server
  // both arguments are optional, with default values being DEFAULT_GSIP and DEFAULT_GSPORT

  int opt;
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;

  // TODO: what kind of error checking do we need to do regarding the arguments?
  while ((opt = getopt(argc, argv, "n:p:")) != -1) {
    switch (opt) {
      case 'n':
        GSIP = optarg;
        break;
      case 'p':
        GSport = optarg;
        break;
      default:
        std::cerr << WRONG_ARGS_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  fd = newSocket(SOCK_DGRAM, GSIP, GSport);
  int res = mkdir("hints", 0700);
  if (res == -1 && errno != EEXIST) {
    // if the directory can't be created and it doesn't already exist
    std::cerr << MKDIR_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  int ret;
  char buffer[MAX_USER_INPUT];
  char response[UDP_RECV_SIZE];
  std::string message;
  memset(buffer, 0, MAX_USER_INPUT);

  // TODO: should we include a help menu as the first thing the player sees?
  // TODO: the while loop contains repeated code in error handling, try to abstract it
  std::cout << "> ";
  while (fgets(buffer, MAX_USER_INPUT, stdin) != NULL) {
    if (strlen(buffer) == 1 && buffer[0] == '\n') {
      // if the user just pressed enter
      memset(buffer, 0, MAX_USER_INPUT);
      std::cout << "> ";
      continue;
    }

    std::string input(buffer);
    std::string command = input.substr(0, input.find(' '));
    message.clear();

    // if command isn't a key in handlePlayerMessage, print error
    if (handlePlayerMessage.find(command) == handlePlayerMessage.end()) {
      std::cerr << "[ERR]: Invalid command. Expected one of: ";
      for (auto it = handlePlayerMessage.begin(); it != handlePlayerMessage.end(); ++it) {
        std::cout << it->first << " ";
      }
      std::cout << std::endl;
      memset(buffer, 0, MAX_USER_INPUT);
      std::cout << "> ";
      continue;
    }

    ret = handlePlayerMessage[command](&message, input);
    if (ret == -1) {
      // error has already been handled, just continue
      memset(buffer, 0, MAX_USER_INPUT);
      std::cout << "> ";
      continue;
    }
    memset(response, 0, UDP_RECV_SIZE);
    ret = exchangeUDPMessage(message, response);
    if (ret == -1) {
      // error has already been handled, just continue
      memset(buffer, 0, MAX_USER_INPUT);
      std::cout << "> ";
      continue;
    }
    ret = parseUDPResponse(response);

    memset(buffer, 0, MAX_USER_INPUT);
    std::cout << "> ";
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  close(fd);
  freeaddrinfo(serverInfo);
  exit(EXIT_SUCCESS);
}
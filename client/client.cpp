#include "client-api.h"
#include "client-protocol.h"

// TODO: standardize messages with macros
// TODO: in order for the program to exit gracefully, we always need to close any open sockets!!

// FIXME: try to find a better way to handle start/sg (etc) w/ the same handler
std::map<std::string, std::function<int(std::string *message, std::string input)>>
    handlePlayerMessage = {{"start", handleStart},
                           {"sg", handleStart},
                           {"play", handlePlay},
                           {"pl", handlePlay},
                           {"guess", handleGuess},
                           {"gw", handleGuess},
                           {"scoreboard", handleScoreboard},
                           {"sb", handleScoreboard},
                           {"hint", handleHint},
                           {"h", handleHint},
                           {"state", handleState},
                           {"st", handleState},
                           {"quit", handleQuit},
                           {"exit", handleExit},
                           {"rev", handleDebug}};

int newSocket(struct addrinfo *serverInfo, int type, std::string addr, std::string port) {
  const int fd = socket(AF_INET, type, 0);
  if (fd == -1) {
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
  return fd;
}

// TODO: handler functions' beginning is very similar, abstract it
int handleStart(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  size_t pos2 = input.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected 2 arguments." << std::endl;
    return -1;
  }
  // plid is a string going from pos+1 to the end of the string, without accounting with \0 and \n
  std::string plid = input.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  if (plid.length() != 6) {
    std::cout << "[ERR]: Invalid PLID. Expected 6 characters." << std::endl;
    return -1;
  }
  std::for_each(plid.begin(), plid.end(), [](char c) {
    if (!std::isdigit(c)) {
      // FIXME: cout below for debug purposes only, remove after
      std::cout << "[ERR]: Invalid PLID. Expected 6 digits but included " << c << std::endl;
      // std::cout << "[ERR]: Invalid PLID. Expected 6 digits." << std::endl;
      return -1;
    }
  });
  playerID = plid;
  *message = "RSG " + plid + "\n";
  // checking if last character is a newline, for debug purposes only
  if (message->back() != '\n') {
    std::cout << "[ERR]: Last character is not a newline. Not sending a message." << std::endl;
    return -1;
  }
  return 0;
}

int handlePlay(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  size_t pos2 = input.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected 2 arguments." << std::endl;
    return -1;
  }
  // check if the second argument is a single letter
  std::string letter = input.substr(pos1 + 1);
  letter.erase(std::remove(letter.begin(), letter.end(), '\n'), letter.end());
  if (letter.length() != 1 || !std::isalpha(letter[0])) {
    std::cout << "[ERR]: Invalid input. Expected a single letter." << std::endl;
    return -1;
  }
  *message = "PLG " + playerID + " " + letter + " " + std::to_string(trials + 1) + "\n";
  play.setLastGuess(letter[0]);
  return 0;
}

int handleGuess(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  size_t pos2 = input.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected 2 arguments." << std::endl;
    return -1;
  }
  std::string guess = input.substr(pos1 + 1);
  guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());
  if (guess.length() != play.getWordLength()) {
    std::cout << "[ERR]: Invalid input. Expected a word of length " << play.getWordLength()
              << std::endl;
    return -1;
  }
  *message = "PWG " + playerID + " " + guess + " " + std::to_string(trials + 1) + "\n";
  return 0;
}

int handleScoreboard(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected no additional arguments." << std::endl;
    return -1;
  }
  *message = "GSB\n";
  return 0;
}

int handleHint(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected no additional arguments." << std::endl;
    return -1;
  }
  *message = "GHL " + playerID + "\n";
  return 0;
}

int handleState(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected no additional arguments." << std::endl;
    return -1;
  }
  *message = "STA " + playerID + "\n";
  return 0;
}

int handleQuit(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected no additional arguments." << std::endl;
    return -1;
  }
  *message = "QUT " + playerID + "\n";
  return 0;
}

int handleDebug(std::string *message, std::string input) {
  size_t pos1 = input.find(' ');
  if (pos1 != std::string::npos) {
    std::cout << "[ERR]: Invalid input. Expected no additional arguments." << std::endl;
    return -1;
  }
  *message = "REV " + playerID + "\n";
  return 0;
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

  while ((opt = getopt(argc, argv, "n:p:")) != -1) {
    switch (opt) {
      case 'n':
        GSIP = std::string(optarg);
        // TODO: it's possible to receive both an IPv4 or a hostname, so we've
        // got to check for both
        break;
      case 'p':
        // TODO: check if the port is valid?
        GSport = std::string(optarg);
        break;
      default:
        std::cerr << "[ERR] Usage: ./player [-n GSIP] [-p GSport]" << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  struct addrinfo *serverInfo = NULL;
  const int fd = newSocket(serverInfo, SOCK_DGRAM, GSIP, GSport);
  if (fd == -1) {
    std::cout << "[ERR]: Failed to create socket. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }

  int res = mkdir("hints", 0700);
  if (res == -1 && errno != EEXIST) {
    // if the directory can't be created and it doesn't already exist
    std::cout << "[ERR]: Failed to create hints directory. Exiting." << std::endl;
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
      std::cout << "[ERR]: Invalid command. Expected one of: ";
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
    ret = exchangeUDPMessage(fd, message, serverInfo, response);
    if (ret == -1) {
      // error has already been handled, just continue
      memset(buffer, 0, MAX_USER_INPUT);
      std::cout << "> ";
      continue;
    }
    ret = parseUDPResponse(response, message);

    memset(buffer, 0, MAX_USER_INPUT);
    std::cout << "> ";
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  close(fd);
  freeaddrinfo(serverInfo);
  exit(EXIT_SUCCESS);
}
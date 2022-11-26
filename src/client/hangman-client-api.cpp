#include "hangman-client-api.h"

// TODO: standardize messages with macros
// TODO: in order for the program to exit gracefully, we always need to close any open sockets!!

std::map<std::string, std::function<int(std::string *message)>> handlePlayerMessage = {
    {"start", handleStart}, {"play", handlePlay},
    {"guess", handleGuess}, {"scoreboard", handleScoreboard},
    {"hint", handleHint},   {"state", handleState},
    {"quit", handleQuit},   {"exit", handleExit},
};

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

  Play play = Play(1, 1); // default constructor
  int ret;
  char buffer[MAX_USER_INPUT];
  char response[UDP_RECV_SIZE];
  std::string message;
  memset(buffer, 0, MAX_USER_INPUT);

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
    ret = handlePlayerMessage[command](&message);
    if (ret == -1) {
      // error has already been handled, just continue
      continue;
    }
    memset(response, 0, UDP_RECV_SIZE);
    ret = exchangeUDPMessage(fd, message, serverInfo, response);
    if (ret == -1) {
      // error has already been handled, just continue
      continue;
    }
    ret = parseUDPResponse(response, message, play);
    if (ret == -1) {
      // error has already been handled, just continue
      continue;
    }

    memset(buffer, 0, MAX_USER_INPUT);
    std::cout << "> ";
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  close(fd);
  freeaddrinfo(serverInfo);
  exit(EXIT_SUCCESS);
}
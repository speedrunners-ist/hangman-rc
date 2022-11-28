#include "common.h"

// Creates a new socket and connects to the server
int newSocket(int type, std::string addr, std::string port, struct addrinfo **serverInfo) {
  int socketFd = socket(AF_INET, type, 0);
  if (socketFd == -1) {
    // FIXME: should we really exit here?
    std::cout << "[ERR]: Failed to create socket. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = type;

  // TODO: see it this works
  int status;
  if (!addr.empty()) {
    status = getaddrinfo(addr.c_str(), port.c_str(), &hints, serverInfo);

  } else {
    status = getaddrinfo(NULL, port.c_str(), &hints, serverInfo);
  }

  if (status != 0) {
    std::cout << "[ERR]: Failed to get address info. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
  return socketFd;
}

// Below, TCP-related functions

// Below, UDP-related functions

// Below, miscellaneous functions

unsigned int initialAvailableMistakes(unsigned int wordLength) {
  if (wordLength <= 6) {
    return 7;
  } else if (7 >= wordLength && wordLength <= 10) {
    return 8;
  }
  return 9;
}

std::string buildMessage(std::vector<std::string> args) {
  // clang-format off
  return std::accumulate(
    ++args.begin(), args.end(), std::string(args[0]),
    [](std::string a, std::string b) { 
      return a + " " + b;
    }
  ).append("\n");
  // clang-format on
}

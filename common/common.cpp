#include "common.h"

// Creates a new socket and connects to the server
int newSocket(int type, std::string addr, std::string port, struct addrinfo *hints,
              struct addrinfo **serverInfo) {
  int socketFd = socket(AF_INET, type, 0);
  if (socketFd == -1) {
    std::cout << "[ERR]: Failed to create socket." << std::endl;
    return -1;
  }
  memset(hints, 0, sizeof *hints);
  hints->ai_family = AF_INET;
  hints->ai_socktype = type;

  // TODO: see it this works
  std::cout << "[INFO]: Connecting to " << addr << ":" << port << std::endl;
  int status;
  if (!addr.empty()) {
    status = getaddrinfo(addr.c_str(), port.c_str(), hints, serverInfo);
    if (status != 0) {
      std::cerr << "[ERR]: Failed to get address info. Exiting." << std::endl;
      return -1;
    }
    return socketFd;
  }
  hints->ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, port.c_str(), hints, serverInfo);

  if (status != 0) {
    // TODO: macro
    std::cerr << "[ERR]: Failed to get address info. Exiting." << std::endl;
    return -1;
  }

  if (bind(socketFd, (*serverInfo)->ai_addr, (*serverInfo)->ai_addrlen) != 0) {
    std::cout << "[ERR]: Failed to bind socket. Exiting." << std::endl;
    exit(EXIT_FAILURE); // TODO standardize exit
  }

  if (!addr.empty())
    return socketFd;

  if (bind(socketFd, (*serverInfo)->ai_addr, (*serverInfo)->ai_addrlen) != 0) {
    std::cout << "[ERR]: Failed to bind socket. Exiting." << std::endl;
    exit(1);
  }
  return socketFd;
}

int turnOnSocketTimer(int socketFd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  tv.tv_sec = SOCKET_TIMEOUT;
  if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << "[ERR]: Failed to set socket timeout. Exiting." << std::endl;
    // FIXME: is this exit graceful?
    exit(EXIT_FAILURE);
  }
  return 0;
}

int turnOffSocketTimer(int socketFd) {
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  if (setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cout << "[ERR]: Failed to reset socket timeout. Exiting." << std::endl;
    // FIXME: is this exit graceful?
    exit(EXIT_FAILURE);
  }
  return 0;
}

// Below, miscellaneous functions

unsigned int initialAvailableMistakes(unsigned int wordLength) {
  if (wordLength <= 6) {
    return 7;
  } else if (7 >= wordLength && wordLength <= 10) {
    return 8;
  }
  return 9;
}

std::string buildSplitString(std::vector<std::string> args) {
  // clang-format off
  return std::accumulate(
    ++args.begin(), args.end(), std::string(args[0]),
    [](std::string a, std::string b) { 
      return a + " " + b;
    }
  ).append("\n");
  // clang-format on
}

int displayFile(std::string fileName, std::string dir) {
  std::ifstream file;
  file.open(dir + "/" + fileName);
  if (!file.is_open()) {
    std::cout << "[ERR]: Failed to open file " << fileName << std::endl;
    return -1;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::cout << line << std::endl;
  }
  file.close();
  return 0;
}

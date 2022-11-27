#include "client-api.h"

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

void exitGracefully(std::string errorMessage) {
  std::cerr << errorMessage << std::endl;

  // TODO: close socket in case of error
  // close(fd);
  // freeaddrinfo(serverInfo);
  // exit(EXIT_SUCCESS);
}

std::string buildPlayerMessage(std::vector<std::string> args) {
  // clang-format off
  return std::accumulate(
    ++args.begin(), args.end(), std::string(args[0]),
    [](std::string a, std::string b) { 
      return a + " " + b;
    }
  ).append("\n");
  // clang-format on
}

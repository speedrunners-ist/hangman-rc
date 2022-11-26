#include "client-protocol.h"

// main function that makes orders
int main(int argc, char *argv[]) {
  trials = 0;
  int opt;
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;

  // TODO: what kind of error checking do we need to do regarding the arguments?

  // Read the command line arguments
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

  // TODO: check error
  newSocket(SOCK_DGRAM, GSIP, GSport);
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
  std::cout << "> ";

  // Read the user input
  while (fgets(buffer, MAX_USER_INPUT, stdin) != NULL) {

    // if the user just pressed enter
    if (strlen(buffer) == 1 && buffer[0] == '\n') {
      continueReading(buffer);
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
      continueReading(buffer);
      continue;
    }

    // if the command is valid, call the appropriate function
    ret = handlePlayerMessage[command](&message, input);
    if (ret == -1) {
      // error has already been handled, just continue
      continueReading(buffer);
      continue;
    }

    // clear response buffer
    memset(response, 0, UDP_RECV_SIZE);
    ret = exchangeUDPMessage(message, response);
    if (ret == -1) {
      // error has already been handled, just continue
      continueReading(buffer);
      continue;
    }

    // get the response from the server
    ret = parseUDPResponse(response);

    continueReading(buffer);
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  // close(fd);
  // freeaddrinfo(serverInfo);
  // exit(EXIT_SUCCESS);
}
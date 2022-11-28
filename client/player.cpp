#include "client-protocol.h"

// clang-format off
static commandHandler handlePlayerMessage = {
  { "start", sendSNG },      { "sg", sendSNG },
  { "play", sendPLG },       { "pl", sendPLG },
  { "guess", sendPWG },      { "gw", sendPWG },
  { "scoreboard", sendGSB }, { "sb", sendGSB },
  { "hint", sendGHL },       { "h", sendGHL },
  { "state", sendSTA },      { "st", sendSTA },
  { "quit", sendQUT },       { "exit", sendQUT },
  { "rev", sendREV }
};
// clang-format on

// Clears the buffer and prints new terminal prompt
void continueReading(char *buffer) {
  memset(buffer, 0, MAX_USER_INPUT);
  std::cout << "> ";
}

// main function that makes orders
int main(int argc, char *argv[]) {
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
  createSocketUDP(GSIP, GSport);
  int res = mkdir("hints", 0700);
  if (res == -1 && errno != EEXIST) {
    // if the directory can't be created and it doesn't already exist
    std::cerr << MKDIR_ERROR << std::endl;
    // TODO: close socket
    exit(EXIT_FAILURE);
  }

  char buffer[MAX_USER_INPUT];

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
    // command is the first word - be it with the input ending in a space or \n,
    // with priority to the space
    std::string command = input.substr(0, input.find_first_of(" \n"));

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
    if (handlePlayerMessage[command](input) == EXIT_HANGMAN) {
      break;
    }
    continueReading(buffer);
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  // TODO: add "closing game" message

  // close(fd);
  // freeaddrinfo(serverInfo);
  // exit(EXIT_SUCCESS);
}

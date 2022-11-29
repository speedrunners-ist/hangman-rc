#include "client-protocol.h"

// clang-format off
commandHandler handlePlayerMessage = {
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

// C++ unfortunately doesn't have a built-in function to retrieve a map's keys
std::vector<std::string> getKeys(commandHandler map) {
  std::vector<std::string> keys;
  for (auto const &pair : map) {
    keys.push_back(pair.first);
  }
  return keys;
}

// main function that makes orders
int main(int argc, char *argv[]) {
  int opt;
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;

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
      const std::string allCommands = buildSplitString(getKeys(handlePlayerMessage));
      std::cerr << UNEXPECTED_COMMAND_ERROR(allCommands);
      continueReading(buffer);
      continue;
    }

    // if the command is valid, call the appropriate function
    if (forceExit(command) || handlePlayerMessage[command](input) == EXIT_HANGMAN) {
      break;
    }
    continueReading(buffer);
  }

  // delete the directory and its contents - should we really do it like this?
  system("rm -rf hints");

  std::cout << "Exiting the program. Thanks for playing!" << std::endl;

  // close(fd);
  // freeaddrinfo(serverInfo);
  // exit(EXIT_SUCCESS);
}

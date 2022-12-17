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

void signalHandler(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectUDP();
  disconnectTCP();
  std::cout << EXIT_PROGRAM << std::endl;
  exit(signum);
}

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

  const peerInfo peer = {GSIP, GSport};
  if (createSocketUDP(peer) == -1) {
    exit(EXIT_FAILURE);
  }

  char buffer[MAX_USER_INPUT];
  printHelpMenu();

  // Read the user input
  while (fgets(buffer, MAX_USER_INPUT, stdin) != NULL) {
    continueReading(buffer);
    // if the user just pressed enter
    if (strlen(buffer) == 1 && buffer[0] == '\n') {
      continue;
    }

    std::string input(buffer);
    std::string command = input.substr(0, input.find_first_of(" \n"));

    // if command isn't a key in handlePlayerMessage, print error
    if (handlePlayerMessage.find(command) == handlePlayerMessage.end()) {
      const std::string allCommands = buildSplitStringNewline(getKeys(handlePlayerMessage));
      std::cerr << UNEXPECTED_COMMAND_ERROR(allCommands);
      continue;
    }

    messageInfo message = {input, peer};
    if (forceExitClient(command) || handlePlayerMessage[command](message) == EXIT_HANGMAN) {
      break;
    }
    displayCurrentInformation();
  }

  std::cout << EXIT_PROGRAM << std::endl;
  if (disconnectUDP() == -1) {
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

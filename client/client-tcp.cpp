#include "client-protocol.h"

static char responseTCP[TCP_READ_SIZE];

// clang-format off
static responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};
// clang-format on

int exchangeTCPMessage(std::string message, char *response) {
  // TODO, w/ placeholders in order to compile
  std::cout << "[INFO]: Sending message: " << message;
  strcpy(response, "TCP response");
  std::cout << response << std::endl;
  return 0;
}

int parseTCPResponse(char *response) {
  // TODO, w/ placeholders in order to compile
  std::cout << "[INFO]: Received response: " << response;
  return 0;
}

int generalTCPHandler(std::string message) {
  memset(responseTCP, 0, TCP_READ_SIZE);
  if (exchangeTCPMessage(message, responseTCP) == -1) {
    return -1;
  }
  return parseTCPResponse(responseTCP);
}

// TODO: implement the 3 handlers below
int handleRSB(struct serverResponse response) { return -1; }

int handleRHL(struct serverResponse response) { return -1; }

int handleRST(struct serverResponse response) { return -1; }

int handleScoreboard(std::string message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  message = "GSB\n";
  return 0;
}

int handleHint(std::string message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  message = "GHL " + playerID + "\n";
  return 0;
}

int handleState(std::string message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  message = "STA " + playerID + "\n";
  return 0;
}

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

// TODO: handlers below only implemented to compile
int handleRSB(struct serverResponse response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int handleRHL(struct serverResponse response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int handleRST(struct serverResponse response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int handleGSB(std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  const std::string message = buildPlayerMessage({"GSB"});
  return 0;
}

int handleGHL(std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  const std::string message = buildPlayerMessage({"GHL", playerID});
  return 0;
}

int handleSTA(std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  const std::string message = buildPlayerMessage({"GST", playerID});
  return 0;
}

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
  if (validateArgsAmount(input, GSB_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildMessage({"GSB"});
  return generalTCPHandler(message);
}

int handleGHL(std::string input) {
  if (validateArgsAmount(input, GHL_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildMessage({"GHL", plid});
  return generalTCPHandler(message);
}

int handleSTA(std::string input) {
  if (validateArgsAmount(input, STA_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildMessage({"GST", plid});
  return generalTCPHandler(message);
}

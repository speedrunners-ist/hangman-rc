#include "client-protocol.h"

static struct addrinfo *serverInfoTCP;
static int socketFdTCP;
static struct addrinfo hints;
static char responseTCP[TCP_READ_SIZE];

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};
// clang-format on

void createSocketTCP(std::string addr, std::string port) {
  socketFdTCP = newSocket(SOCK_STREAM, addr, port, &hints, &serverInfoTCP);
}

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
int handleRSB(struct protocolMessage response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int handleRHL(struct protocolMessage response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int handleRST(struct protocolMessage response) {
  std::cout << "[INFO]: Received response: " << response.body;
  return 0;
}

int sendGSB(std::string input) {
  if (validateArgsAmount(input, SCOREBOARD_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GSB"});
  return generalTCPHandler(message);
}

int sendGHL(std::string input) {
  if (validateArgsAmount(input, HINT_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GHL", plid});
  return generalTCPHandler(message);
}

int sendSTA(std::string input) {
  if (validateArgsAmount(input, STATE_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GST", plid});
  return generalTCPHandler(message);
}

#include "client-protocol.h"

static char responseTCP[TCP_READ_SIZE];

// TODO: create exchange and parse functions for TCP

int generalTCPHandler(std::string message) {
  memset(responseTCP, 0, TCP_READ_SIZE);
  if (exchangeTCPMessage(message, responseTCP) == -1) {
    return -1;
  }
  return parseTCPResponse(responseTCP);
}

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

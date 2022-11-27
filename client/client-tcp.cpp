#include "client-api.h"

int handleScoreboard(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "GSB\n";
  return 0;
}

int handleHint(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "GHL " + playerID + "\n";
  return 0;
}

int handleState(std::string *message, std::string input) {
  if (validateSingleArgCommand(input) == -1) {
    return -1;
  }
  *message = "STA " + playerID + "\n";
  return 0;
}

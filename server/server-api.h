#ifndef SERVER_API_H
#define SERVER_API_H

#include "common/common.h"
#include <algorithm>
#include <functional>

typedef std::map<std::string, std::function<int(std::string *message, std::string input)>>
    messageHandler;

// Player message handlers
// TODO: try to find a better way to handle functions with two arguments
int handleStart(std::string *message, std::string input);
int handlePlay(std::string *message, std::string input);
int handleGuess(std::string *message, std::string input);
int handleScoreboard(std::string *message, std::string input);
int handleHint(std::string *message, std::string input);
int handleState(std::string *message, std::string input);
int handleQuit(std::string *message, std::string input);
int handleExit(std::string *message, std::string input);
int handleDebug(std::string *message, std::string input);

static messageHandler handlePlayerMessageServer = {{"start", handleStart},
                                                   {"sg", handleStart},
                                                   {"play", handlePlay},
                                                   {"pl", handlePlay},
                                                   {"guess", handleGuess},
                                                   {"gw", handleGuess},
                                                   {"scoreboard", handleScoreboard},
                                                   {"sb", handleScoreboard},
                                                   {"hint", handleHint},
                                                   {"h", handleHint},
                                                   {"state", handleState},
                                                   {"st", handleState},
                                                   {"quit", handleQuit},
                                                   {"exit", handleExit},
                                                   {"rev", handleDebug}};

#endif /* SERVER_API_H */
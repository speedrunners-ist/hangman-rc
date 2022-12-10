#ifndef SERVER_API_H
#define SERVER_API_H

#include "server-utils.h"

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./GS file-path [-p GSport] -v"

// User Messages
#define STARTING_SERVER "Starting server..."
#define STARTING_SERVER_ERROR "[ERR]: Failed to set server parameters. Exiting..."
#define VERBOSE_SUCCESS(host, service) "[INFO]: Message sent by [" << host << ":" << service << "]"
#define VERBOSE_ERROR(error) "[ERR]: getnameinfo: " << gai_strerror(error)

// RSG return codes
#define CREATE_GAME_ERROR 1
#define CREATE_GAME_SUCCESS 2

// RLG return codes
#define SUCCESS_GUESS 1
#define SUCCESS_FINAL_GUESS 2
#define DUPLICATE_GUESS 3
#define WRONG_GUESS 4
#define WRONG_FINAL_GUESS 5
#define TRIAL_MISMATCH 6
#define SYNTAX_ERROR 7

// RQT return codes
#define CLOSE_GAME_ERROR 1
#define CLOSE_GAME_SUCCESS 2

#define SCOREBOARD_EMPTY 1
#define SCOREBOARD_SUCCESS 2

#define GAME_SCORE(correct, total) (correct * 100 / total)

GameState createGame(int length, int mistakes, std::string playerID);
int getAvailableMistakes(GameState play);
std::string getWord(GameState play);
int playCorrectGuess(GameState &play, std::string positions, int n);
void playIncorrectGuess(GameState &play);
void playCorrectFinalGuess(GameState &play);
void playCorrectFinalWordGuess(GameState &play);
void setLastGuess(GameState &play, char guess);
void setLastWordGuess(GameState &play, std::string guess);
int getWordLength(GameState play);
void setPlayerID(GameState &play, std::string id);
std::string getPlayerID(GameState play);
void incrementTrials(GameState &play);
int getTrials(GameState play);

int setupWordList(std::string filePath);
bool isOngoingGame(std::string plid);
std::pair<std::string, std::string> getRandomLine();

int createGameSession(std::string plid, std::string &arguments);
int retrieveGame(std::string playerID, GameState &state);
int playLetter(std::string plid, std::string letter, std::string trial, std::string &positions);
int getLetterOccurrences(std::string word, char letter, std::string &positions);
int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments);
int closeGameSession(std::string plid);
int insertScore(std::string plid, GameState state);

int getScoreboard(std::string &scoreboard);

#endif /* SERVER_API_H */

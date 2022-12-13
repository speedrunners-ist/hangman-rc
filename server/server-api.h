#ifndef SERVER_API_H
#define SERVER_API_H

#include "server-utils.h"

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./GS file-path [-p GSport] -v"

// User Messages
#define STARTING_SERVER "[INFO] Starting server."
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

#define SCOREBOARD_ERROR 0
#define SCOREBOARD_EMPTY 1
#define SCOREBOARD_SUCCESS 2

#define HINT_ERROR 1
#define HINT_SUCCESS 2

#define STATE_ONGOING 1
#define STATE_FINISHED 2
#define STATE_ERROR 3

// File storing utilities + messages
#define EMPTY_FILE(file) "[ERR]: File " + file + " is empty."
#define UNEXPECTED_GAME_LINE(line) "[ERR]: Unexpected line in game file: " + line
#define GAME_SCORE(correct, total) (correct * 100 / total)
#define CORRECT_LETTER "Correct guess (letter):"
#define CORRECT_FINAL_LETTER "Correct final guess (letter):"
#define CORRECT_FINAL_WORD "Correct final guess (word):"
#define WRONG_LETTER "Wrong guess (letter):"
#define WRONG_WORD "Wrong guess (word):"
#define WRONG_FINAL_LETTER "Wrong final guess (letter):"
#define WRONG_FINAL_WORD "Wrong final guess (word):"
#define QUIT_GAME "Game quit by player."

GameState createGame(int length, int mistakes, std::string playerID);
int getAvailableMistakes(GameState state);
std::string getWord(GameState state);
void playCorrectLetterGuess(GameState &state, std::string letter);
void playIncorrectLetterGuess(GameState &state, std::string letter);
void playIncorrectWordGuess(GameState &state, std::string word);
int getWordLength(GameState state);
void setPlayerID(GameState &state, std::string id);
std::string getPlayerID(GameState state);
void incrementTrials(GameState &state);
int getTrials(GameState state);

int setupWordList(std::string filePath);
bool isOngoingGame(std::string plid);
std::pair<std::string, std::string> getRandomLine();

int createGameSession(std::string plid, std::string &arguments);
int retrieveGame(std::string playerID, GameState &state);
int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments);
int getLetterOccurrences(std::string word, char letter);
int getLetterOccurrencesPositions(std::string word, char letter, std::string &positions);
int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments);
int closeGameSession(std::string plid);
int insertScore(std::string plid, GameState &state);

int getScoreboard(std::string &response);
int getHint(std::string plid, std::string &response, std::string &filePath);
int getState(std::string plid, std::string &response, std::string &filePath);

#endif /* SERVER_API_H */

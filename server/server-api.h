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

enum {
  // RSG return codes
  CREATE_GAME_ERROR,
  CREATE_GAME_SUCCESS,

  // RLG return codes
  SYNTAX_ERROR,
  TRIAL_MISMATCH,
  DUPLICATE_GUESS,
  WRONG_GUESS,
  WRONG_FINAL_GUESS,
  SUCCESS_GUESS,
  SUCCESS_FINAL_GUESS,

  // RQT return codes
  CLOSE_GAME_ERROR,
  CLOSE_GAME_SUCCESS,

  // RSB return codes
  SCOREBOARD_ERROR,
  SCOREBOARD_EMPTY,
  SCOREBOARD_SUCCESS,

  // RHL return codes
  HINT_ERROR,
  HINT_SUCCESS,

  // RST return codes
  STATE_ERROR,
  STATE_FINISHED,
  STATE_ONGOING
};

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
int getAvailableMistakes(GameState play);
std::string getWord(GameState play);
void playCorrectLetterGuess(GameState &state, std::string letter);
void playIncorrectLetterGuess(GameState &state, std::string letter);
void playIncorrectWordGuess(GameState &state, std::string word);
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

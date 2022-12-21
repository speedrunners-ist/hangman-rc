#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "common.h"

// Path to the directory where games are stored
#define GAMES_DIR "server/assets/games/"

// Path to the directory where a player's finished games are stored
#define PLID_GAMES_DIR(plid) GAMES_DIR + plid + "/"

// File name of a player's ongoing game
#define PLID_GAME(plid) "GAME_" + plid + ".txt"

// File name of a player'game
#define GAME_FILENAME(plid) "STATE_" + plid + ".txt"

// Path to a player's ongoing game
#define ONGOING_GAMES_PATH(plid) std::string(GAMES_DIR) + PLID_GAME(plid)

// Path to a player's finished game, given the time it was finished
#define FINISHED_GAMES_PATH(plid, time) PLID_GAMES_DIR(plid) + "GAME_" + time + ".txt"

// Path to the scoreboard file
#define SCORES_PATH "server/assets/scores/scoreboard.txt"

// Header for the scoreboard file
#define TOP_10_HEADER "TOP 10 SCORES"

// Header for the scoreboard file's table
#define SCORES_HEADER "SCORE PLAYER  WORD      GOOD TRIALS  TOTAL TRIALS"

// Format for the player's games to be stored in
#define TIME_FORMAT "%Y-%m-%d_%H-%M-%S"

// Maximum amount of characters in a TIME_FORMAT string
#define MAX_TIME_FORMAT 64

// Path to the directory where hints are stored
#define HINTS_PATH(filename) "server/assets/hints/" + filename

// Path to the directory where the state of a player's game is temporarily stored
#define TMP_DIR "server/assets/state/"

// Path to the temporary file for a player's game
#define TMP_PATH(plid) std::string(TMP_DIR) + "STATE_" + plid + ".txt"

// Message to be written in the state temp file if there haven't been any plays yet
#define NO_GUESSES "No guesses have been made yet."

/**
 * @brief Creates a file for a player's game.
 *
 * @param plid The player's ID.
 * @param word The word to be guessed.
 * @param hint The hint for the word.
 * @return 0 if successful, -1 otherwise.
 */
int createGameFile(std::string plid, std::string word, std::string hint);

/**
 * @brief Appends a play to a player's game file.
 *
 * @param plid The player's ID.
 * @param code Description of the play.
 * @param play The guess made by the player.
 * @return 0 if successful, -1 otherwise.
 */
int appendGameFile(std::string plid, std::string code, std::string play);

/**
 * @brief Transfers a player's ongoing game to the finished games directory.
 *
 * @param plid The player's ID.
 * @return 0 if successful, -1 otherwise.
 */
int transferGameFile(std::string plid);

/**
 * @brief Appends a score to the scoreboard file.
 *
 * @param score The score to be appended.
 * @param scoreline The line to be appended.
 * @return 0 if successful, -1 otherwise. Here, success doesn't necessarily mean that the score was appended,
 * but rather that the files were handled without errors.
 */
int appendScoreFile(int score, std::string scoreline);

/**
 * @brief Writes the header for the scoreboard file.
 *
 * @param file The file to be written to.
 * @param lines The header lines to be written.
 */
void writeScoreFileHeader(std::fstream &file, std::vector<std::string> lines);

/**
 * @brief Gets the last finished game of a player.
 *
 * @param plid The player's ID.
 * @param filePath The path to the last finished game (where it is stored).
 * @return 0 if successful, -1 otherwise.
 */
int getLastFinishedGame(std::string plid, std::string &filePath);

/**
 * @brief Auxiliar function to create a temporary file for a player's game state.
 *
 * @param plid The player's ID.
 * @param filePath The path to the temporary file.
 * @return 0 if successful, -1 otherwise.
 */
int createPlaceholderState(std::string plid, std::string filePath);

/**
 * @brief Destroys all temporary files on exit.
 */
void destroyTempFiles();

#endif

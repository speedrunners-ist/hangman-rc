#ifndef SERVER_API_H
#define SERVER_API_H

#include "server/include/server-utils.h"

// Wrong program arguments
#define WRONG_ARGS_ERROR "[ERR] Usage: ./GS file-path [-p GSport] -v"

// Server initialization message
#define STARTING_SERVER "[INFO] Starting server."

// Server initialization error
#define STARTING_SERVER_ERROR "[ERR]: Failed to set server parameters. Exiting..."

// Verbose success message for a message sent by a client
#define VERBOSE_SUCCESS(protocol, host, service)                                                             \
  "[INFO]: " << protocol << " message sent by [" << host << ":" << service << "]"

// Verbose error message for a message sent by a client
#define VERBOSE_ERROR(error) "[ERR]: getnameinfo: " << gai_strerror(error)

// Return codes for each specific scenario for a message received by the server
enum {
  // RSG return codes
  CREATE_GAME_ERROR,   // RSG ERR
  CREATE_GAME_ONGOING, // RSG NOK
  CREATE_GAME_SUCCESS, // RSG OK

  // RLG and RWG return codes
  
  GENERAL_ERROR,       // ERR
  SYNTAX_ERROR,        // RLG ERR/RWG ERR
  TRIAL_MISMATCH,      // RLG INV/RWG INV
  DUPLICATE_GUESS,     // RLG DUP
  WRONG_GUESS,         // RLG NOK/RWG NOK
  WRONG_FINAL_GUESS,   // RLG OVR/RWG OVR
  SUCCESS_GUESS,       // RLG OK/RWG WIN
  SUCCESS_FINAL_GUESS, // RLG WIN

  // RQT return codes
  CLOSE_GAME_ERROR,   // RQT ERR
  CLOSE_GAME_SUCCESS, // RQT OK

  // RRV return codes
  REVEAL_ERROR,   // RRV ERR
  REVEAL_SUCCESS, // RRV OK

  // RSB return codes
  SCOREBOARD_ERROR,   // ERR
  SCOREBOARD_EMPTY,   // RSB EMPTY
  SCOREBOARD_SUCCESS, // RSB OK

  // RHL return codes
  HINT_NOK,     // RHL NOK
  HINT_SUCCESS, // RHL OK

  // RST return codes
  STATE_NOK,      // RST NOK
  STATE_FINISHED, // RST FIN
  STATE_ONGOING   // RST ACT
};

// Below, file handling utility macros + scoreboard line macros

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
#define HINT "Received hint file:"
#define QUIT_GAME "Game quit by player."

// Below, a series of wrapper functions which allow the server not to be aware of any
// "global" state's internal structure at any given time

/**
 * @brief Gets the available mistakes for a given game state.
 *
 * @param state The game state.
 * @return The number of available mistakes.
 */
int getAvailableMistakes(GameState state);

/**
 * @brief Gets the word for a given game state.
 *
 * @param state The game state.
 * @return The word.
 */
std::string getWord(GameState state);

/**
 * @brief Gets the word length for a given game state.
 *
 * @param state The game state.
 * @return The word length.
 */
int getWordLength(GameState state);

/**
 * @brief Sets the playerID for a given game state.
 *
 * @param state The game state.
 * @param plid The playerID.
 */
void setPlayerID(GameState &state, std::string plid);

/**
 * @brief Gets the playerID for a given game state.
 *
 * @param state The game state.
 * @return The playerID.
 */
std::string getPlayerID(GameState state);

/**
 * @brief Gets the number of trials for a given game state.
 *
 * @param state The game state.
 * @return The number of trials.
 */
int getTrials(GameState state);

/**
 * @brief Increments the trials for a given game state in a single unit.
 *
 * @param state The game state.
 */
void incrementTrials(GameState &state);

/**
 * @brief Handles the case in which a letter is guessed correctly.
 *
 * @param state The game state.
 * @param letter The guessed letter.
 */
void playCorrectLetterGuess(GameState &state, std::string letter);

/**
 * @brief Handles the case in which a letter is guessed incorrectly.
 *
 * @param state The game state.
 * @param letter The guessed letter.
 */
void playIncorrectLetterGuess(GameState &state, std::string letter);

/**
 * @brief Handles the case in which a word is guessed correctly.
 *
 * @param state The game state.
 * @param word The guessed word.
 */
void playIncorrectWordGuess(GameState &state, std::string word);

/**
 * @brief Handles the case in a hint is retrieved.
 *
 * @param state The game state.
 * @param hint The hint.
 */
void setHint(GameState &state, std::string hint);

/**
 * @brief Displays the peer's information (useful for verbose scenarios).
 * 
 * @param res The addrinfo structure containing the peer's information.
 * @param connection The type of connection (TCP or UDP).
 */
void displayPeerInfo(struct addrinfo *res, std::string connection);

/**
 * @brief Sets up the server for the retrieval of a word-hint pair from a word list.
 *
 * @param filePath The path to the word list file.
 * @return 0 if the setup was successful, -1 otherwise.
 */
int setupWordList(std::string filePath);

/**
 * @brief Checks if a given game session is ongoing.
 *
 * @param plid The playerID of the game session.
 * @return true if the game session is ongoing, false otherwise.
 */
bool isOngoingGame(std::string plid);

/**
 * @brief Returns a word-hint pair from the given word list, according to
 * compilation-defined rules (randomly or sequential).
 * 
 * @return The word-hint pair according to previously defined rules.
*/
std::pair<std::string, std::string> getWordHintPair();

/**
 * @brief Gets the number of occurrences of a given letter in a given word.
 *
 * @param word The word.
 * @param letter The letter.
 * @return The number of occurrences.
 */
int getLetterOccurrences(std::string word, char letter);

/**
 * @brief Gets the positions of a given letter in a given word.
 *
 * @param word The word.
 * @param letter The letter.
 * @param positions The positions.
 * @return The number of occurrences.
 */
int getLetterOccurrencesPositions(std::string word, char letter, std::string &positions);

/**
 * @brief Retrieves a game state's information from a file.
 *
 * @param plid The playerID of the game session.
 * @param state The game state.
 * @return 0 if the retrieval was successful, -1 otherwise.
 */
int retrieveGame(std::string playerID, GameState &state);

/**
 * @brief Creates a new game session.
 *
 * @param plid The playerID of the game session.
 * @param arguments The arguments for the game session - word, hint, number of mistakes.
 * @return CREATE_GAME_SUCCESS if the game session was created successfully, CREATE_GAME_ERROR otherwise.
 */
int createGameSession(std::string plid, std::string &arguments);

/**
 * @brief Plays a letter guess in a given game session.
 *
 * @param plid The playerID of the game session.
 * @param letter The letter.
 * @param trial The trial number.
 * @param arguments The arguments to be sent back to the client.
 * @return A plethora of possible return values.
 */
int playLetter(std::string plid, std::string letter, std::string trial, std::string &arguments);

/**
 * @brief Plays a word guess in a given game session.
 *
 * @param plid The playerID of the game session.
 * @param word The word.
 * @param trial The trial number.
 * @param arguments The arguments to be sent back to the client.
 * @return A plethora of possible return values.
 */
int guessWord(std::string plid, std::string word, std::string trial, std::string &arguments);

/**
 * @brief Closes a given game session, saving its state to a file.
 *
 * @param plid The playerID of the game session.
 * @return 0 if the game session was closed successfully, -1 otherwise.
 */
int closeGameSession(std::string plid);

/**
 * @brief Retrieves a given game session's correct word in development, and "OK" in production.
 * Used for testing purposes.
 *
 * @param plid The playerID of the game session.
 * @param word The wanted word.
 * @return REVEAL_SUCCESS if the word was retrieved successfully, REVEAL_ERROR otherwise.
 */
int revealWord(std::string plid, std::string &word);

/**
 * @brief Calculates the score for a given game session, tries to insert it into the scoreboard.
 *
 * @param plid The playerID of the game session.
 * @param state The game state.
 * @return The score.
 */
int insertScore(std::string plid, GameState &state);

/**
 * @brief Gets the scoreboard.
 *
 * @param response The message to be sent back to the client.
 * @return A plethora of possible return values.
 */
int getScoreboard(std::string &response);

/**
 * @brief Gets the hint for a given game session.
 *
 * @param plid The playerID of the game session.
 * @param response The message to be sent back to the client.
 * @param filePath The path to the hint file.
 * @return HINT_SUCCESS if the hint was retrieved successfully, HINT_ERROR otherwise.
 */
int getHint(std::string plid, std::string &response, std::string &filePath);

/**
 * @brief Gets the state for a given game session.
 *
 * @param plid The playerID of the game session.
 * @param response The message to be sent back to the client.
 * @param filePath The path to the state file.
 * @return A plethora of possible return values.
 */
int getState(std::string plid, std::string &response, std::string &filePath);

#endif /* SERVER_API_H */

// In production, a compiler flag (PRODUCTION) will be used in order to tell that
// the RRV command should answer with RRV OK, not RRV with the actual word.
#ifdef PRODUCTION
#define RRV_OK(word) "OK"
#else
#define RRV_OK(word) word
#endif

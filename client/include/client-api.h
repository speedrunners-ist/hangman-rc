#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common.h"

/**
 * @brief Creates a GameState instance, with the given parameters.
 *
 * @param args The arguments (length and mistakes) required to create a new game state.
 * @param playerID The player's ID.
 */
void createGame(std::vector<int> args, std::string playerID);

/**
 * @brief Alters the GameState instance to reflect a correct letter guess.
 *
 * @param positions The positions of the letter in the word.
 * @param n The number of times the letter appears in the word.
 * @return 0 if no errors occurred, -1 otherwise.
 */
int playCorrectGuess(std::string positions, int n);

/**
 * @brief Alters the GameState instance to reflect an incorrect letter guess.
 */
void playIncorrectGuess();

/**
 * @brief Alters the GameState instance to reflect a correct final letter guess.
 */
void playCorrectFinalGuess();

/**
 * @brief Alters the GameState instance to reflect a correct final word guess.
 *
 * @return 0 if no errors occurred, -1 otherwise.
 */
void playCorrectFinalWordGuess();

/**
 * @brief Gets the number of mistakes available to the player.
 *
 * @return The number of allowed mistakes.
 */
int getAvailableMistakes();

/**
 * @brief Gets the word as has been guessed so far.
 *
 * @return The word as currently constructed.
 */
std::string getWord();

/**
 * @brief Sets the last letter guessed. Utilized mainly before sending a message,
 * since the server's response doesn't include the last letter guessed.
 *
 * @param guess The last letter guessed.
 */
void setLastGuess(char guess);

/**
 * @brief Sets the last word guessed. Utilized mainly before sending a message,
 * since the server's response doesn't include the last word guessed.
 *
 * @param guess The last word guessed.
 */
void setLastWordGuess(std::string guess);

/**
 * @brief Gets the length of the word to be guessed.
 *
 * @return The length of the word.
 */
int getWordLength();

/**
 * @brief Sets the player's ID.
 *
 * @param id The player's ID.
 */
void setPlayerID(std::string id);

/**
 * @brief Gets the player's ID.
 *
 * @return The player's ID.
 */
std::string getPlayerID();

/**
 * @brief Resets the GameState instance, setting it to inactive.
 */
void resetGame();

/**
 * @brief Checks whether the game is active.
 *
 * @return True if the game is active, false otherwise.
 */
bool isActive();

/**
 * @brief Increments the player's trial count in one unit.
 */
void incrementTrials();

/**
 * @brief Gets the number of trials the player has made.
 *
 * @return The number of trials.
 */
int getTrials();

/**
 * @brief Checks whether the player wants to exit the program, and if so, exits
 *
 * @param command The command to be checked.
 * @return True if the player wants to exit, false otherwise.
 */
bool forceExitClient(std::string command);

/**
 * @brief Prints the help menu.
 */
void printHelpMenu();
/**
 * @brief Displays the current information regarding the current game state.
 */
void displayCurrentInformation();

#endif /* CLIENT_API_H */

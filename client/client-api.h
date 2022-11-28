#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common/common.h"
#include <algorithm>

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define SOCKET_ERROR "[ERR]: Failed to create socket. Exiting."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info. Exiting."
#define MKDIR_ERROR "[ERR]: Failed to create hints directory. Exiting."
#define DIFF_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR(length)                                                        \
  "[ERR]: Invalid input. Expected a word of length " + std::to_string(length) + "."

class GameState {
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
  char lastGuess;
  std::map<char, bool> guessedLetters;
  std::map<int, char> word;

public:
  GameState(int length, int mistakes) {
    this->wordLength = length;
    this->mistakesLeft = mistakes;
    for (int i = 0; i < length; i++) {
      word[i] = '_';
    }
    for (char c = 'a'; c <= 'z'; c++) {
      guessedLetters[c] = false;
    }
  }

  int getAvailableMistakes() { return mistakesLeft; }

  char getLastGuess() { return lastGuess; }

  int getWordLength() { return wordLength; }

  std::string getWord() {
    std::string wordStr;
    for (int i = 0; i < wordLength; i++) {
      wordStr += word[i];
    }
    return wordStr;
  }

  void setLastGuess(char guess) { lastGuess = guess; }

  void setWord(std::map<int, char> newWord) { this->word = newWord; }

  void incorrectGuess() {
    char guess = getLastGuess();
    // TODO: do we have to check if we're setting to true something that's already true?
    guessedLetters[guess] = true;
    guessesMade++;
    mistakesLeft--;
  }

  int correctGuess(std::string positions, int n) {
    char guess = getLastGuess();
    // for every int in positions, set the corresponding (-1) char in word to guess
    // done with std::string.find
    std::map<int, char> initialWord = std::map<int, char>(word);
    size_t pos = positions.find(' ');
    int readPositions = 0;
    while (pos != std::string::npos) {
      int wordPosition = std::stoi(positions.substr(0, pos)) - 1;
      if (wordPosition < 0 || wordPosition >= wordLength) {
        std::cerr << "[ERR]: Server response includes invalid positions." << std::endl;
        setWord(initialWord);
        return -1;
      }
      // TODO: check if the position is already filled
      word[wordPosition] = guess;
      positions = positions.substr(pos + 1);
      readPositions++;
    }
    if (n != readPositions) {
      // the answer didn't include as many positions as expected
      std::cerr << "[ERR]: Expected a different amount of positions than the ones given."
                << std::endl;
      setWord(initialWord);
      return -1;
    }
    guessesMade++;
    guessedLetters[guess] = true;
    return 0;
  }

  void correctFinalGuess() {
    char guess = getLastGuess();
    guessedLetters[guess] = true;
    for (int i = 0; i < wordLength; i++) {
      if (word[i] == '_') {
        word[i] = guess;
      }
    }
    guessesMade++;
  }
};

void createGame(int length, int mistakes);
int getAvailableMistakes();
std::string getWord();
int playCorrectGuess(std::string positions, int n);
void playIncorrectGuess();
void playCorrectFinalGuess();
void setLastGuess(char guess);
int getWordLength();

int validateArgsAmount(std::string input, int n);
int validatePlayerID(std::string playerID);
void exitGracefully(std::string errorMessage);
void continueReading(char *buffer);

#endif /* CLIENT_API_H */

#include "../hangman-common.h"
#include <algorithm>

class Play {
  unsigned int wordLength;
  unsigned int mistakesLeft;
  unsigned int guessesMade = 0;
  char lastGuess;
  std::map<char, bool> guessedLetters;
  std::map<unsigned int, char> word;

public:
  Play(int wordLength, int mistakesLeft) {
    this->wordLength = wordLength;
    this->mistakesLeft = mistakesLeft;
    for (int i = 0; i < wordLength; i++) {
      word[i] = '_';
    }
    for (char c = 'a'; c <= 'z'; c++) {
      guessedLetters[c] = false;
    }
  }

  unsigned int getAvailableMistakes() { return mistakesLeft; }

  char getLastGuess() { return lastGuess; }

  std::string getWord() {
    std::string wordStr;
    for (int i = 0; i < wordLength; i++) {
      wordStr += word[i];
    }
    return wordStr;
  }

  void setLastGuess(char guess) { lastGuess = guess; }

  void setWord(std::map<unsigned int, char> word) { this->word = word; }

  void incorrectGuess() {
    char guess = getLastGuess();
    // TODO: do we have to check if we're setting to true something that's already true?
    guessedLetters[guess] = true;
    guessesMade++;
    mistakesLeft--;
  }

  int correctGuess(std::string positions) {
    char guess = getLastGuess();
    guessedLetters[guess] = true;
    // for every int in positions, set the corresponding (-1) char in word to guess
    // done with std::string.find
    std::map<unsigned int, char> initialWord = std::map<unsigned int, char>(word);
    size_t pos = positions.find(' ');
    while (pos != std::string::npos) {
      int wordPosition = std::stoi(positions.substr(0, pos)) - 1;
      if (wordPosition < 0 || wordPosition >= wordLength) {
        std::cout << "[ERR]: Server response includes invalid positions." << std::endl;
        setWord(initialWord);
        return -1;
      }
      // TODO: check if the position is already filled
      word[wordPosition] = guess;
      positions = positions.substr(pos + 1);
      guessesMade++;
    }
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
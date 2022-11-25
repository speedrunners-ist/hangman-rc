#include <../hangman-common.h>

class Play {
  unsigned int wordLength;
  unsigned int mistakesLeft;
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

  std::string getWord() {
    std::string wordStr;
    for (int i = 0; i < wordLength; i++) {
      wordStr += word[i];
    }
    return wordStr;
  }

  void guessLetter(char c) {
    if (guessedLetters[c]) {
      // FIXME: this isn't the right error
      std::cout << "You already guessed that letter!" << std::endl;
      return;
    }
    guessedLetters[c] = true;
    bool found = false;
    for (int i = 0; i < wordLength; i++) {
      if (word[i] == c) {
        found = true;
        break;
      }
    }
    if (!found) {
      mistakesLeft--;
    }
  }
};
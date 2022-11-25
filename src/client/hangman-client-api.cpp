#include <../hangman-common.h>

class Play {
  unsigned int wordLength;
  unsigned int mistakesLeft;
  std::map<char, bool> guessedLetters;
  std::map<unsigned int, char> word;

public:
  Play(int wordLength) {
    this->wordLength = wordLength;
    this->mistakesLeft = initialAvailableMistakes(wordLength);
    for (int i = 0; i < wordLength; i++) {
      word[i] = '_';
    }
    for (char c = 'a'; c <= 'z'; c++) {
      guessedLetters[c] = false;
    }
  }

  unsigned int getAvailableMistakes() { return mistakesLeft; }

  void printWord() {
    for (int i = 0; i < wordLength; i++) {
      std::cout << word[i] << " ";
    }
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

int main(int argc, char *argv[]) {
  // command format: ./player [-n GSIP] [-p GSport]
  // GSIP: IP address of the game server
  // GSport: port number of the game server
  // both arguments are optional, with default values being DEFAULT_GSIP and DEFAULT_GSPORT

  int opt;
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;

  while ((opt = getopt(argc, argv, "n:p:")) != -1) {
    switch (opt) {
      case 'n':
        GSIP = optarg;
        break;
      case 'p':
        GSport = optarg;
        break;
      default:
        std::cout << "Usage: ./player [-n GSIP] [-p GSport]" << std::endl;
        return 1;
    }
  }

  exit(0);
}
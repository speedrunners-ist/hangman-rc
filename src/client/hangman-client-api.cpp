#include <../hangman-common.h>

class Play {
  unsigned int wordLength;
  unsigned int guessesLeft;
  std::map<char, bool> guessedLetters;
  std::map<unsigned int, char> word;

public:
  Play(int wordLength) {
    this->wordLength = wordLength;
    this->guessesLeft = initialGuesses(wordLength);
    for (int i = 0; i < wordLength; i++) {
      word[i] = '_';
    }
    for (char c = 'a'; c <= 'z'; c++) {
      guessedLetters[c] = false;
    }
  }

  unsigned int getRemainingGuesses() { return guessesLeft; }

  void printWord() {
    for (int i = 0; i < wordLength; i++) {
      std::cout << word[i] << " ";
    }
  }
};

int main(int argc, char *argv[]) {
  // command format: ./player [-n GSIP] [-p GSport]
  // GSIP: IP address of the game server
  // GSport: port number of the game server
  // If no arguments are provided, use default values: DEFAULT_GSIP and DEFAULT_GSPORT

  int opt;
  std::string GSIP;
  std::string GSport;

  if (argc == 1) {
    GSIP = DEFAULT_GSIP;
    GSport = DEFAULT_GSPORT;
  } else {
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
      switch (opt) {
        case 'n':
          GSIP = optarg;
          break;
        case 'p':
          GSport = optarg;
          break;
        default:
          std::cerr << "[ERR] Usage: " << argv[0] << " [-n GSIP] [-p GSport]" << std::endl;
          exit(EXIT_FAILURE);
      }
    }
  }

  exit(0);
}
#include <../hangman-common.h>

class Play {
  int wordLength;
  int guessesLeft;
  std::map<char, bool> guessedLetters;
  std::map<int, char> word;

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

    int getRemainingGuesses() {
      return guessesLeft;
    }

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

  // If no arguments are provided, use default values

  // parse command line arguments
  int opt;
  std::string GSIP;
  std::string GSport;

  // TODO: missing case for when no arguments are provided
  while ((opt = getopt(argc, argv, "n:p:")) != -1) {
    switch (opt) {
      case 'n':
        GSIP = optarg;
        break;
      case 'p':
        GSport = optarg;
        break;
      default:
        std::cerr << "Usage: " << argv[0] << " [-n GSIP] [-p GSport]" << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  exit(0);
}
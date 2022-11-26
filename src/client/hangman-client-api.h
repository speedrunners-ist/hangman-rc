#include "../hangman-common.h"
#include <algorithm>
#include <functional>

class Play {
  unsigned int wordLength;
  unsigned int mistakesLeft;
  unsigned int guessesMade = 0;
  char lastGuess;
  std::map<char, bool> guessedLetters;
  std::map<int, char> word;

public:
  Play(unsigned int length, unsigned int mistakes) {
    this->wordLength = length;
    this->mistakesLeft = mistakes;
    for (int i = 0; i < length; i++) {
      word[i] = '_';
    }
    for (char c = 'a'; c <= 'z'; c++) {
      guessedLetters[c] = false;
    }
  }

  unsigned int getAvailableMistakes() { return mistakesLeft; }

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
        std::cout << "[ERR]: Server response includes invalid positions." << std::endl;
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
      std::cout << "[ERR]: Expected a different amount of positions than the ones given."
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

// Global variables - the current game state and current player ID
// perhaps we should consider a different way to store these?
Play play = Play(1, 1); // default constructor
std::string playerID;
int trials = 0;

// Player message handlers
// TODO: try to find a better way to handle functions with two arguments
int handleStart(std::string *message, std::string input);
int handlePlay(std::string *message, std::string input);
int handleGuess(std::string *message, std::string input);
int handleScoreboard(std::string *message, std::string input);
int handleHint(std::string *message, std::string input);
int handleState(std::string *message, std::string input);
int handleQuit(std::string *message, std::string input);
int handleExit(std::string *message, std::string input);

// UDP socket functions
int exchangeUDPMessage(int fd, std::string message, struct addrinfo *serverAddr, char *response);
int parseUDPResponse(char *response, std::string &message);

// TCP socket functions
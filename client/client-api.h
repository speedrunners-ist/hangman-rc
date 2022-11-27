#ifndef CLIENT_API_H
#define CLIENT_API_H

#include "common/common.h"
#include <algorithm>
#include <functional>

typedef std::map<std::string, std::function<int(std::string *message, std::string input)>>
    messageHandler;

// Error Messages
#define WRONG_ARGS_ERROR "[ERR] Usage: ./player [-n GSIP] [-p GSport]"
#define SOCKET_ERROR "[ERR]: Failed to create socket. Exiting."
#define GETADDRINFO_ERROR "[ERR]: Failed to get address info. Exiting."
#define MKDIR_ERROR "[ERR]: Failed to create hints directory. Exiting."
#define TOO_MANY_ARGS_ERROR "[ERR]: Invalid input. Expected different number of arguments."
#define INVALID_PLID_LEN_ERROR "[ERR]: Invalid PLID. Expected 6 characters."
#define INVALID_PLID_CHAR_ERROR "[ERR]: Invalid PLID. Expected 6 digits."
#define EXPECTED_LETTER_ERROR "[ERR]: Invalid input. Expected a single letter."
#define EXPECTED_WORD_DIF_LEN_ERROR "[ERR]: Invalid input. Expected a word of length "
// UDP Error Messages - should we really include RSG/RLG/... here? It shouldn't be
// something the player should know about, I think
#define SENDTO_ERROR "[ERR]: Failed to send message to server."
#define RECVFROM_ERROR "[ERR]: Failed to receive message from server."
#define UDP_RESPONSE_ERROR "[ERR]: Response from server does not match the UDP protocol."
#define UDP_HANGMAN_ERROR "[ERR]: Response from server does not match any expected protocols."
#define RSG_ERROR "[ERR]: Response from server does not match RSG protocol."
#define RLG_ERROR "[ERR]: Response from server does not match RLG protocol."
#define RLG_INVALID_WORD_LEN "[ERR]: Response from server includes invalid word length."
// Messages shown to the user
#define RSG_OK(mistakes, word)                                                                     \
  ("New game started (max " + std::to_string(mistakes) +                                           \
   " mistakes allowed). Word to guess: " + word)
#define RSG_NOK "Failed to start a new game. Try again later."
#define RLG_WIN(word) ("WELL DONE! You guessed: " + word)
#define RLG_DUP "You have already guessed this letter."
#define RLG_NOK(mistakes) ("Wrong guess. " + std::to_string(mistakes) + " errors left.")
#define RLG_OVR "GAME OVER! You do not have any more errors left."
#define RLG_INV "An invalid trial parameter was sent. Try again."
#define RLG_ERR "RLG ERR"

class Play {
  int wordLength;
  int mistakesLeft;
  int guessesMade = 0;
  char lastGuess;
  std::map<char, bool> guessedLetters;
  std::map<int, char> word;

public:
  Play(int length, int mistakes) {
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

int validateSingleArgCommand(std::string input);
int validateTwoArgsCommand(std::string input);
void exitGracefully(std::string errorMessage);
void continueReading(char *buffer);

// Global variables - the current game state and current player ID
// perhaps we should consider a different way to store these?
static Play play = Play(1, 1);
static std::string playerID;

#endif /* CLIENT_API_H */

// TODO: add client-side functions for actions requiring UDP
// start, play, guess, exit
#include "hangman-client-api.h"

int exchangeUDPMessage(int fd, std::string message, struct addrinfo *serverAddr, char *response) {
  unsigned int triesLeft = UDP_TRIES;
  do {
    // note: we don't send the null terminator, hence the -1
    if (sendto(fd, message.c_str(), message.length() - 1, 0, serverAddr->ai_addr,
               serverAddr->ai_addrlen) == -1) {
      std::cout << "[ERR]: Failed to send message." << std::endl;
      return -1;
    }

    socklen_t addrLen = sizeof(serverAddr->ai_addr);
    ssize_t bytesReceived = recvfrom(fd, response, UDP_RECV_SIZE, 0, serverAddr->ai_addr, &addrLen);
    if (bytesReceived == -1) {
      if (triesLeft == 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
        std::cout << "[ERR]: Failed to receive response." << std::endl;
        return -1;
      }
      continue;
    }

    if (response[bytesReceived - 1] != '\n') {
      std::cout << "[ERR]: Response does not match the UDP protocol." << std::endl;
      return -1;
    }
    response[bytesReceived - 1] = '\0';
    return 0;

  } while (--triesLeft >= 0);

  std::cout << "[ERR]: Failed to receive response." << std::endl;
  return -1;
}

// make sure we test formatting for every parameter in every response
int parseUDPResponse(char *response, std::string &message, Play &play) {
  std::string responseStr(response);
  size_t pos1 = responseStr.find(' ');
  size_t pos2 = responseStr.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  if (code == "RSG") {
    if (status == "OK") {
      size_t pos3 = responseStr.find(' ', pos2 + 1);
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      if (pos3 == std::string::npos || pos4 == std::string::npos) {
        std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
        return -1;
      }

      // TODO: check if the word length is valid?
      play = Play(std::stoi(responseStr.substr(pos2 + 1, pos3 - pos2 - 1)),
                  std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1)));
      std::cout << "New game started (max " << play.getAvailableMistakes()
                << " errors): " << play.getWord() << std::endl;
      return 0;
    } else if (status == "NOK") {
      std::cout << "Failed to start a new game. Try again later" << std::endl;
      return 0;
    } else {
      // unknown status
      std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
      return -1;
    }
  } else if (code == "RLG") {
    size_t pos3 = responseStr.find(' ', pos2 + 1);
    if (pos3 == std::string::npos) {
      std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
      return -1;
    }
    if (status == "OK") {
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      size_t pos5 = responseStr.find(' ', pos4 + 1);
      if (pos4 == std::string::npos || pos5 == std::string::npos) {
        std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
        return -1;
      }
      int n = std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1));
      if (n < 3 || n > 30) {
        std::cout << "[ERR]: Server response does not match any protocol." << std::endl;
        return -1;
      }
      if (play.correctGuess(responseStr.substr(pos4 + 1), n) == 0) {
        return 0;
      }
    } else if (status == "WIN") {
      play.correctFinalGuess();
      std::cout << "WELL DONE! You guessed: " << play.getWord() << std::endl;
      return 0;
    } else if (status == "DUP") {
      std::cout << "You have already guessed this letter." << std::endl;
      return 0;
    } else if (status == "NOK") {
      play.incorrectGuess();
      std::cout << "Wrong guess. " << play.getAvailableMistakes() << " errors left." << std::endl;
      return 0;
    } else if (status == "OVR") {
      play.incorrectGuess();
      std::cout << "GAME OVER! You do not have any more errors left. The word was: "
                << play.getWord() << std::endl;
      return 0;
    } else if (status == "INV") {
      std::cout << "An invalid trial parameter was sent. Try again." << std::endl;
    } else if (status == "ERR") {
      std::cout << "RLG ERR" << std::endl;
    }
  } else if (code == "RWG") {
  } // TODO: implement the rest
  return -1;
}
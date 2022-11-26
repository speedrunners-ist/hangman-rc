// TODO: add client-side functions for actions requiring UDP
// start, play, guess, exit
#include "hangman-client-api.h"

int exchangeUDPMessage(int fd, std::string message, struct addrinfo *serverAddr, char *response) {
  unsigned int triesLeft = UDP_TRIES;
  do {
    // note: we don't send the null terminator, hence the -1
    if (sendto(fd, message.c_str(), message.length() - 1, 0, serverAddr->ai_addr,
               serverAddr->ai_addrlen) == -1) {
      std::cerr << SENDTO_ERROR << std::endl;
      return -1;
    }

    socklen_t addrLen = sizeof(serverAddr->ai_addr);
    ssize_t bytesReceived = recvfrom(fd, response, UDP_RECV_SIZE, 0, serverAddr->ai_addr, &addrLen);
    if (bytesReceived == -1) {
      if (triesLeft == 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
        break;
      }
      continue;
    }

    if (response[bytesReceived - 1] != '\n') {
      std::cerr << UDP_RESPONSE_ERROR << std::endl;
      return -1;
    }
    response[bytesReceived - 1] = '\0';
    return 0;

  } while (--triesLeft >= 0);

  std::cerr << RECVFROM_ERROR << std::endl;
  return -1;
}

// make sure we test formatting for every parameter in every response
int parseUDPResponse(char *response, std::string &message) {
  std::string responseStr(response);
  size_t pos1 = responseStr.find(' ');
  size_t pos2 = responseStr.find(' ', pos1 + 1);
  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  if (code == "RSG") {
    if (status == "OK") {
      size_t pos3 = responseStr.find(' ', pos2 + 1);
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      if (pos3 == std::string::npos || pos4 == std::string::npos) {
        std::cerr << RSG_ERROR << std::endl;
        return -1;
      }

      // TODO: check if the word length is valid?
      play = Play(std::stoi(responseStr.substr(pos2 + 1, pos3 - pos2 - 1)),
                  std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1)));
      std::cout << RSG_OK(play.getAvailableMistakes(), play.getWord()) << std::endl;
      return 0;
    } else if (status == "NOK") {
      std::cout << RSG_NOK << std::endl;
      return 0;
    }
    // unknown status
    std::cerr << RSG_ERROR << std::endl;
  } else if (code == "RLG") {
    size_t pos3 = responseStr.find(' ', pos2 + 1);
    if (pos3 == std::string::npos) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    if (status == "OK") {
      size_t pos4 = responseStr.find(' ', pos3 + 1);
      size_t pos5 = responseStr.find(' ', pos4 + 1);
      if (pos4 == std::string::npos || pos5 == std::string::npos) {
        std::cerr << RLG_ERROR << std::endl;
        return -1;
      }
      int n = std::stoi(responseStr.substr(pos3 + 1, pos4 - pos3 - 1));
      if (n < 3 || n > 30) {
        std::cerr << RLG_INVALID_WORD_LEN << std::endl;
        return -1;
      }
      if (play.correctGuess(responseStr.substr(pos4 + 1), n) == 0) {
        trials++;
        return 0;
      }
    } else if (status == "WIN") {
      play.correctFinalGuess();
      std::cout << RLG_WIN(play.getWord()) << std::endl;
      trials++;
      return 0;
    } else if (status == "DUP") {
      std::cout << RLG_DUP << std::endl;
      return 0;
    } else if (status == "NOK") {
      play.incorrectGuess();
      std::cout << RLG_NOK(play.getAvailableMistakes()) << std::endl;
      trials++;
      return 0;
    } else if (status == "OVR") {
      play.incorrectGuess();
      std::cout << RLG_OVR << std::endl;
      trials++;
      return 0;
    } else if (status == "INV") {
      std::cout << RLG_INV << std::endl;
    } else if (status == "ERR") {
      std::cout << RLG_ERR << std::endl;
    }
  } else if (code == "RWG") {
    // TODO: don't forget to increment trials here
  } // TODO: implement the rest
  return -1;
}
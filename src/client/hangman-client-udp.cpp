// TODO: add client-side functions for actions requiring UDP
// start, play, guess, exit
#include <../hangman-common.h>

int exchangeMessageUDP(int fd, std::string message, struct addrinfo *serverAddr, char *response) {
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

  } while (--triesLeft > 0);

  std::cout << "[ERR]: Failed to receive response." << std::endl;
  return -1;
}
#include "server-protocol.h"

static struct addrinfo hintsTCP, *resTCP;
static int socketFdTCP, newfd;
static socklen_t addrlen;
static ssize_t n, nw;
static char *ptr, buffer[128];

void createSocketTCP(std::string addr, std::string port) {
  int errcode;
  socketFdTCP = newSocket(SOCK_STREAM, addr, port, &hintsTCP, &resTCP);
  if (socketFdTCP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (listen(socketFdTCP, 5) == -1) /*error*/
    exit(1);

  // TODO: fix loop
  while (1) {
    addrlen = sizeof(addr);
    if ((newfd = accept(socketFdTCP, (struct sockaddr *)&addr, &addrlen)) == -1)
      /*error*/ exit(1);
    while ((n = read(newfd, buffer, 128)) != 0) {
      if (n == -1) /*error*/
        exit(1);
      ptr = &buffer[0];
      while (n > 0) {
        if ((nw = write(newfd, ptr, n)) <= 0) /*error*/
          exit(1);
        n -= nw;
        ptr += nw;
      }
    }
    close(newfd);
  }
}
// Server message handlers
int handleGSB(struct protocolMessage message) { return 0; }
int handleGHL(struct protocolMessage message) { return 0; }
int handleSTA(struct protocolMessage message) { return 0; }

// TCP server message Send
int sendRSB(std::string input) { return 0; }
int sendRHL(std::string input) { return 0; }
int sendRST(std::string input) { return 0; }

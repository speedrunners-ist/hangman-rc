#include "server-protocol.h"

static struct addrinfo hints, *res;
static int fd, newfd, errcode;
static struct sockaddr_in addr;
static socklen_t addrlen;
static ssize_t n;
static char buffer[128];

void openTCP(std::string GSport) {
  const char *GSPORT = GSport.c_str();

  if (fd = socket(AF_INET, SOCK_STREAM, 0 == -1)) // TCP socket
    exit(1);                                      // error

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP socket
  hints.ai_flags = AI_PASSIVE;

  if ((errcode = getaddrinfo(NULL, GSPORT, &hints, &res)) != 0)
    exit(1); /*error*/
  if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
    exit(1); /*error*/

  // TODO: change number of connections to Constant
  if (listen(fd, 5) == -1)
    exit(1); /*error*/

  while (1) {
    addrlen = sizeof(addr);
    if ((newfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1)
      exit(1); /*error*/

    if (read(newfd, buffer, 128) == -1) /*error*/
      exit(1);

    write(1, "received: ", 10);
    write(1, buffer, n);

    if (write(newfd, buffer, n) == -1) /*error*/
      exit(1);

    close(newfd);
  }
  freeaddrinfo(res);
  close(fd);
}

// Server message handlers
int handleGSB(struct protocolMessage message) { return 0; }
int handleGHL(struct protocolMessage message) { return 0; }
int handleSTA(struct protocolMessage message) { return 0; }

// TCP server message Send
int sendRSB(std::string input) { return 0; }
int sendRHL(std::string input) { return 0; }
int sendRST(std::string input) { return 0; }
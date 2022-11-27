#include "server-protocol.h"

static struct addrinfo hints, *res;
static int fd, errcode;
static struct sockaddr_in addr;
static socklen_t addrlen;
static ssize_t n, nread;
static char buffer[128];

void openUDP(std::string GSport) {
  const char *GSPORT = GSport.c_str();

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    exit(1); // error

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP socket
  hints.ai_flags = AI_PASSIVE;

  if ((errcode = getaddrinfo(NULL, GSPORT, &hints, &res)) != 0) /*error*/
    exit(1);
  if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) /*error*/
    exit(1);

  // Listen for incoming connections
  while (1) {
    addrlen = sizeof(addr);
    if (recvfrom(fd, buffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addr, &addrlen) == -1) /*error*/
      exit(1);

    std::cout << buffer << std::endl;

    if (sendto(fd, buffer, nread, 0, (struct sockaddr *)&addr, addrlen) == -1) /*error*/
      exit(1);
  }

  freeaddrinfo(res);
  close(fd);
  exit(0);
}

// UDP server message Send
int sendRSG(std::string input) { return 0; }
int sendRLG(std::string input) { return 0; }
int sendRWG(std::string input) { return 0; }
int sendRQT(std::string input) { return 0; }
int sendRRV(std::string input) { return 0; }

// Server message handlers
int handleSNG(struct clientRequest message) { return 0; }
int handlePLG(struct clientRequest message) { return 0; }
int handlePWG(struct clientRequest message) { return 0; }
int handleQUT(struct clientRequest message) { return 0; }
int handleREV(struct clientRequest message) { return 0; }

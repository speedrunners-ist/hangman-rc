#include "server-protocol.h"

static struct addrinfo hints, *res;
static int fd, errcode;
static struct sockaddr_in addr;
static socklen_t addrlen;
static ssize_t n, nread;
static char buffer[UDP_RECV_SIZE];
static char responseUDP[UDP_RECV_SIZE];

// clang-format off
static getHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

int parseUDPResponse(char *response) {
  const std::string responseStr(response);
  const size_t pos1 = responseStr.find(' ');
  const size_t pos2 = responseStr.find('\n', pos1 + 1);

  if (pos1 == std::string::npos || pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  const struct clientRequest clientRequest = {code, pos1, status, pos2, responseStr};

  std::cout << "Code: " << code << std::endl;

  return handleUDPClientMessage[code](clientRequest);
}

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

    parseUDPResponse(buffer);

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
int handleSNG(struct clientRequest message) {
  sendRSG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handlePLG(struct clientRequest message) {
  sendRLG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handlePWG(struct clientRequest message) {
  sendRWG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleQUT(struct clientRequest message) {
  sendRQT(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleREV(struct clientRequest message) {
  sendRRV(message.code);
  std::cout << message.code << std::endl;
  return 0;
}

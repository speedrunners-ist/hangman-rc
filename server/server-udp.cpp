#include "server-protocol.h"

static struct addrinfo *serverInfo;
static int socketFd;
static char responseUDP[UDP_RECV_SIZE];
static socklen_t addrlen;
static struct sockaddr_in addrClient;
static char buffer[UDP_RECV_SIZE];
static ssize_t nread;

// clang-format off
static commandHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

void createSocketUDP(std::string addr, std::string port) {
  socketFd = newSocket(SOCK_DGRAM, addr, port, &serverInfo);

  // Listen for incoming connections
  while (1) {

    addrlen = sizeof(addrClient);
    if (recvfrom(socketFd, buffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addrClient, &addrlen) ==
        -1) /*error*/
      exit(1);

    // parseUDPResponse(buffer);

    std::cout << "[INFO]: Received message: " << buffer << std::endl;

    if (sendto(socketFd, buffer, nread, 0, (struct sockaddr *)&addrClient, addrlen) == -1) /*error*/
      exit(1);
  }
}

// UDP server message Send
int sendRSG(std::string input) { return 0; }
int sendRLG(std::string input) { return 0; }
int sendRWG(std::string input) { return 0; }
int sendRQT(std::string input) { return 0; }
int sendRRV(std::string input) { return 0; }

// Server message handlers
int handleSNG(struct protocolMessage message) {
  sendRSG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handlePLG(struct protocolMessage message) {
  sendRLG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handlePWG(struct protocolMessage message) {
  sendRWG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleQUT(struct protocolMessage message) {
  sendRQT(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleREV(struct protocolMessage message) {
  sendRRV(message.code);
  std::cout << message.code << std::endl;
  return 0;
}

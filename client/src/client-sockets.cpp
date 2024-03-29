#include "client-protocol.h"

socketInfo socketTCP, socketUDP;
std::string expectedMessage;

void signalHandler(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnect(getSocket(SOCK_DGRAM));
  disconnect(getSocket(SOCK_STREAM));
  std::cout << EXIT_PROGRAM << std::endl;
  exit(signum);
}

int createSocket(__socket_type type, peerInfo peer, sighandler_t handler) {
  socketInfo socket = handleSocketCreation(type, peer, handler, true);
  if (type == SOCK_DGRAM) {
    socketUDP = socket;
    turnOnSocketTimer(socketUDP.fd);
    return socketUDP.fd;
  }
  socketTCP = socket;
  return socketTCP.fd;
}

int disconnect(socketInfo socket) {
  if (socket.type == SOCK_STREAM) {
    if (!socketTCP.isConnected) {
      return 0;
    }
    socketTCP.isConnected = false;
  } else {
    if (!socketUDP.isConnected) {
      return 0;
    }
    if (!getPlayerID().empty()) {
      sendUDPMessage(buildSplitStringNewline({"QUT", getPlayerID()}), socket.res, socket.fd);
    }
    socketUDP.isConnected = false;
  }
  return disconnectSocket(socket);
}

socketInfo getSocket(__socket_type type) { return type == SOCK_STREAM ? socketTCP : socketUDP; }
struct addrinfo *getServerInfoUDP() {
  return socketUDP.res;
}
struct addrinfo *getServerInfoTCP() {
  return socketTCP.res;
}
int getSocketFdUDP() { return socketUDP.fd; }
int getSocketFdTCP() { return socketTCP.fd; }
std::string getExpectedMessage() { return expectedMessage; }
void setExpectedMessage(std::string message) { expectedMessage = message; }

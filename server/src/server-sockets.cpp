#include "server/include/server-protocol.h"

// here, socketTCP is the TCP socket of the parent process

socketInfo socketTCP, socketUDP;
bool verbose;

int createSocket(__socket_type type, peerInfo peer, sighandler_t handler) {
  socketInfo socket = handleSocketCreation(type, peer, handler, false);
  if (type == SOCK_DGRAM) {
    socketUDP = socket;
    return socketUDP.fd;
  }
  socket.isConnected = true;
  socketTCP = socket;
  return socketTCP.fd;
}

int disconnect(socketInfo socket) {
  if (socket.type == SOCK_STREAM) {
    socketTCP.isConnected = false;
  }
  return disconnectSocket(socket.res, socket.fd);
}

int setServerParameters(std::string filePath, bool vParam) {
  verbose = vParam;
  return setupWordList(filePath);
}

bool checkVerbose() { return verbose; }

socketInfo getSocket(__socket_type type) { return type == SOCK_STREAM ? socketTCP : socketUDP; }
struct addrinfo *getResUDP() { return socketUDP.res; }
struct addrinfo *getResTCP() { return socketTCP.res; }
int getSocketFdUDP() { return socketUDP.fd; }
int getSocketFdTCP() { return socketTCP.fd; }

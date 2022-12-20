#include "server-protocol.h"

socketInfo socketTCP, socketUDP;

int disconnect(socketInfo socket) {
  if (socket.type == SOCK_STREAM) {
    socketTCP.isConnected = false;
  } else {
    // sendUDPMessage(buildSplitStringNewline({"QUT", getPlayerID()}), socket.res, socket.fd);
  }
  return disconnectSocket(socket.res, socket.fd);
}

socketInfo getSocket(__socket_type type) { return type == SOCK_STREAM ? socketTCP : socketUDP; }
struct addrinfo *getServerInfoUDP() { return socketUDP.res; }
struct addrinfo *getServerInfoTCP() { return socketTCP.res; }

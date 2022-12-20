#include "client-protocol.h"

socketInfo socketTCP, socketUDP;

// Creation and retrieval of client socket information (both UDP and TCP)

int createSocket(__socket_type type, peerInfo peer) {
  socketInfo socket;
  socket.fd = newSocket(type, peer, &socket.hints, &socket.res);
  if (socket.fd == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (type == SOCK_STREAM) {
    socket.isConnected = true;
    if (connect(socket.fd, socket.res->ai_addr, socket.res->ai_addrlen) == -1) {
      std::cerr << TCP_SERVER_ERROR << std::endl;
      disconnect(socket);
      return -1;
    }
  }

  if (turnOnSocketTimer(socket.fd) == -1) {
    disconnect(socket);
    return -1;
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  memset(&socket.act, 0, sizeof(socket.act));
  socket.act.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &socket.act, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    disconnect(socket);
    return -1;
  }
  socket.type = type;
  if (type == SOCK_STREAM) {
    socket.isConnected = true;
    socketTCP = socket;
    return socketTCP.fd;
  }
  socketUDP = socket;
  return socket.fd;
}

int disconnect(socketInfo socket) {
  if (socket.type == SOCK_STREAM) {
    socketTCP.isConnected = false;
  } else {
    sendUDPMessage(buildSplitStringNewline({"QUT", getPlayerID()}), socket.res, socket.fd);
  }
  return disconnectSocket(socket.res, socket.fd);
}

socketInfo getSocket(__socket_type type) { return type == SOCK_STREAM ? socketTCP : socketUDP; }
struct addrinfo *getServerInfoUDP() { return socketUDP.res; }
struct addrinfo *getServerInfoTCP() { return socketTCP.res; }
int getSocketFdUDP() { return socketUDP.fd; }
int getSocketFdTCP() { return socketTCP.fd; }
std::string getExpectedMessageUDP() { return socketUDP.expectedMessage; }
std::string getExpectedMessageTCP() { return socketTCP.expectedMessage; }
void setExpectedMessageUDP(std::string message) { socketUDP.expectedMessage = message; }
void setExpectedMessageTCP(std::string message) { socketTCP.expectedMessage = message; }

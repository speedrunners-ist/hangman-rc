#include "client-protocol.h"

struct addrinfo *serverInfoTCP;
struct addrinfo hintsTCP;
int socketFdTCP;
bool isTCPConnected = false;
struct sigaction actTCP;
std::string expectedMessageTCP;

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};

// clang-format on

int createSocketTCP(peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer, &hintsTCP, &serverInfoTCP);
  if (socketFdTCP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    return -1;
  }

  isTCPConnected = true;
  if (connect(socketFdTCP, serverInfoTCP->ai_addr, serverInfoTCP->ai_addrlen) == -1) {
    std::cerr << TCP_SERVER_ERROR << std::endl;
    disconnectTCP();
    return -1;
  }

  if (turnOnSocketTimer(socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  memset(&actTCP, 0, sizeof(actTCP));
  actTCP.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actTCP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    disconnectTCP();
    return -1;
  }
  return socketFdTCP;
}

int disconnectTCP() {
  if (isTCPConnected) {
    isTCPConnected = false;
    return disconnectSocket(serverInfoTCP, socketFdTCP);
  }
  return 0;
}

int generalTCPHandler(std::string message, peerInfo peer) {
  protocolMessage serverMessage;
  if (createSocketTCP(peer) == -1) {
    return -1;
  }

  if (sendTCPMessage(message, serverInfoTCP, socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  }
  if (receiveTCPMessage(serverMessage.body, TCP_DEFAULT_ARGS, socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  }
  if (parseMessage(serverMessage.body, serverMessage, false) == -1) {
    disconnectTCP();
    return -1;
  }

  messageTCPHandler(serverMessage, handleTCPServerMessage);
  return disconnectTCP();
}

int handleRSB(protocolMessage response) {
  if (response.request != expectedMessageTCP) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    if (receiveTCPFile(info, SB_DIR, socketFdTCP) == -1) {
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    return displayFile(SB_PATH(info.fileName));
  }
  if (response.status == "EMPTY") {
    std::cout << RSB_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRHL(protocolMessage response) {
  if (response.request != expectedMessageTCP) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    const int bytesRead = receiveTCPFile(info, H_DIR, socketFdTCP);
    if (bytesRead == -1) {
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << RHL_SUCCESS(info.fileName, bytesRead) << std::endl;
    return 0;
  }
  if (response.status == "NOK") {
    std::cout << RHL_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRST(protocolMessage response) {
  if (response.request != expectedMessageTCP) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "NOK") {
    std::cout << RST_NOK << std::endl;
    return 0;
  } else if (response.status == "ERR") {
    std::cerr << RST_ERR << std::endl;
    return -1;
  }

  fileInfo info;
  const int bytesRead = receiveTCPFile(info, ST_DIR, socketFdTCP);
  if (bytesRead == -1) {
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  if (response.status == "ACT") {
    std::cout << RST_ACT << std::endl;
  } else if (response.status == "FIN") {
    std::cout << RST_FIN << std::endl;
  }
  return displayFile(ST_PATH(info.fileName));
}

int sendGSB(messageInfo info) {
  if (!validArgsAmount(info.input, SCOREBOARD_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GSB"});
  expectedMessageTCP = "RSB";
  return generalTCPHandler(message, info.peer);
}

int sendGHL(messageInfo info) {
  if (!validArgsAmount(info.input, HINT_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GHL", getPlayerID()});
  expectedMessageTCP = "RHL";
  return generalTCPHandler(message, info.peer);
}

int sendSTA(messageInfo info) {
  if (!validArgsAmount(info.input, STATE_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"STA", getPlayerID()});
  expectedMessageTCP = "RST";
  return generalTCPHandler(message, info.peer);
}

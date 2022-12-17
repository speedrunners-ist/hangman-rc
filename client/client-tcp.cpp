#include "client-protocol.h"

struct addrinfo *serverInfoTCP;
struct addrinfo hintsTCP;
int socketFdTCP;
bool isTCPConnected = false;
struct sigaction actTCP;

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};
// clang-format on

int createSocketTCP(peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer, &hintsTCP, &serverInfoTCP);
  if (connect(socketFdTCP, serverInfoTCP->ai_addr, serverInfoTCP->ai_addrlen) == -1) {
    std::cerr << TCP_SERVER_ERROR << std::endl;
    return -1;
  }

  if (turnOnSocketTimer(socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  }

  isTCPConnected = true;
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

int parseTCPResponse(protocolMessage &serverMessage) {
  std::string responseBegin = serverMessage.body;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string status = responseBegin.substr(0, responseBegin.find_first_of(" \n"));
  serverMessage.first = command;
  serverMessage.second = status;
  return messageTCPHandler(socketFdTCP, serverInfoTCP, serverMessage, handleTCPServerMessage);
}

int generalTCPHandler(std::string message, peerInfo peer) {
  protocolMessage serverMessage;
  if (createSocketTCP(peer) == -1) {
    return -1;
  }

  if (sendTCPMessage(message, serverInfoTCP, socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  } else if (receiveTCPMessage(serverMessage.body, TCP_DEFAULT_ARGS, socketFdTCP) == -1) {
    disconnectTCP();
    return -1;
  }
  parseTCPResponse(serverMessage);
  return disconnectTCP();
}

int handleRSB(protocolMessage response) {
  if (response.second == "OK") {
    fileInfo info;
    if (receiveTCPFile(info, SB_DIR, socketFdTCP) == -1) {
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    return displayFile(SB_PATH(info.fileName));
  } else if (response.second == "EMPTY") {
    std::cout << RSB_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRHL(protocolMessage response) {
  if (response.second == "OK") {
    fileInfo info;
    const int bytesRead = receiveTCPFile(info, H_DIR, socketFdTCP);
    if (bytesRead == -1) {
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << RHL_SUCCESS(info.fileName, bytesRead) << std::endl;
    return 0;
  } else if (response.second == "NOK") {
    std::cout << RHL_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRST(protocolMessage response) {
  if (response.second == "NOK") {
    std::cout << RST_NOK << std::endl;
    return 0;
  } else if (response.second == "ERR") {
    std::cerr << RST_ERR << std::endl;
    return -1;
  }

  fileInfo info;
  const int bytesRead = receiveTCPFile(info, ST_DIR, socketFdTCP);
  if (bytesRead == -1) {
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  if (response.second == "ACT") {
    std::cout << RST_ACT << std::endl;
  } else if (response.second == "FIN") {
    std::cout << RST_FIN << std::endl;
  }
  displayFile(ST_PATH(info.fileName));
  return 0;
}

int sendGSB(messageInfo info) {
  if (!validArgsAmount(info.input, SCOREBOARD_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GSB"});
  return generalTCPHandler(message, info.peer);
}

int sendGHL(messageInfo info) {
  if (!validArgsAmount(info.input, HINT_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GHL", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

int sendSTA(messageInfo info) {
  if (!validArgsAmount(info.input, STATE_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"STA", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

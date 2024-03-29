#include "client-protocol.h"

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};

// clang-format on

int generalTCPHandler(std::string message, peerInfo peer) {
  protocolMessage serverMessage;
  if (createSocket(SOCK_STREAM, peer, (sighandler_t)signalHandler) == -1) {
    return -1;
  }
  if (sendTCPMessage(message, getServerInfoTCP(), getSocketFdTCP()) == -1) {
    disconnect(getSocket(SOCK_STREAM));
    return -1;
  }
  if (receiveTCPMessage(serverMessage.body, TCP_DEFAULT_ARGS, getSocketFdTCP()) == -1) {
    disconnect(getSocket(SOCK_STREAM));
    return -1;
  }
  if (parseMessage(serverMessage.body, serverMessage, false) == -1) {
    disconnect(getSocket(SOCK_STREAM));
    return -1;
  }

  messageTCPHandler(serverMessage, handleTCPServerMessage);
  return disconnect(getSocket(SOCK_STREAM));
}

int handleRSB(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    const int bytesRead = receiveTCPFile(info, SB_DIR, getSocketFdTCP());
    if (bytesRead == -1) {
      std::cerr << DISPLAY_ERR << std::endl;
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << RSB_SUCCESS(info.filePath, bytesRead) << std::endl;
    return displayFile(info.filePath);
  }
  if (response.status == "EMPTY") {
    std::cout << RSB_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRHL(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    const int bytesRead = receiveTCPFile(info, H_DIR, getSocketFdTCP());
    if (bytesRead == -1) {
      std::cerr << DISPLAY_ERR << std::endl;
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << RHL_SUCCESS(info.filePath, bytesRead) << std::endl;
    return 0;
  }
  if (response.status == "NOK") {
    std::cout << RHL_FAIL << std::endl;
    return 0;
  }
  return -1;
}

int handleRST(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
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
  const int bytesRead = receiveTCPFile(info, ST_DIR, getSocketFdTCP());
  if (bytesRead == -1) {
    std::cerr << DISPLAY_ERR << std::endl;
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  std::cout << RST_SUCCESS(info.filePath, bytesRead) << std::endl;
  if (response.status == "ACT") {
    std::cout << RST_ACT << std::endl;
  } else if (response.status == "FIN") {
    std::cout << RST_FIN << std::endl;
    resetGame();
  }
  return displayFile(info.filePath);
}

int sendGSB(messageInfo info) {
  if (!validArgsAmount(info.input, SCOREBOARD_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GSB"});
  setExpectedMessage("RSB");
  return generalTCPHandler(message, info.peer);
}

int sendGHL(messageInfo info) {
  if (!validArgsAmount(info.input, HINT_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GHL", getPlayerID()});
  setExpectedMessage("RHL");
  return generalTCPHandler(message, info.peer);
}

int sendSTA(messageInfo info) {
  if (!validArgsAmount(info.input, STATE_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  const std::string message = buildSplitStringNewline({"STA", getPlayerID()});
  setExpectedMessage("RST");
  return generalTCPHandler(message, info.peer);
}

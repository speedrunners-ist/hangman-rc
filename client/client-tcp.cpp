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
  if (createSocket(SOCK_STREAM, peer) == -1) {
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
  if (response.request != getExpectedMessageTCP()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    if (receiveTCPFile(info, SB_DIR, getSocketFdTCP()) == -1) {
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
  if (response.request != getExpectedMessageTCP()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    fileInfo info;
    const int bytesRead = receiveTCPFile(info, H_DIR, getSocketFdTCP());
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
  if (response.request != getExpectedMessageTCP()) {
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
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  if (response.status == "ACT") {
    std::cout << RST_ACT << std::endl;
  } else if (response.status == "FIN") {
    std::cout << RST_FIN << std::endl;
    resetGame();
  }
  return displayFile(ST_PATH(info.fileName));
}

int sendGSB(messageInfo info) {
  if (!validArgsAmount(info.input, SCOREBOARD_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GSB"});
  setExpectedMessageTCP("RSB");
  return generalTCPHandler(message, info.peer);
}

int sendGHL(messageInfo info) {
  if (!validArgsAmount(info.input, HINT_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GHL", getPlayerID()});
  setExpectedMessageTCP("RHL");
  return generalTCPHandler(message, info.peer);
}

int sendSTA(messageInfo info) {
  if (!validArgsAmount(info.input, STATE_ARGS)) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"STA", getPlayerID()});
  setExpectedMessageTCP("RST");
  return generalTCPHandler(message, info.peer);
}

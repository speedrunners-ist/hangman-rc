#include "client-protocol.h"

struct addrinfo *serverInfoTCP;
struct addrinfo hintsTCP;
int socketFdTCP;

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};
// clang-format on

int createSocketTCP(struct peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer.addr, peer.port, &hintsTCP, &serverInfoTCP);
  if (connect(socketFdTCP, serverInfoTCP->ai_addr, serverInfoTCP->ai_addrlen) == -1) {
    std::cerr << TCP_SERVER_ERROR << std::endl;
    return -1;
  }
  return socketFdTCP;
}

int exchangeTCPMessage(std::string message, struct protocolMessage &serverMessage, int args) {
  if (serverInfoTCP == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::string responseMessage;
  if (sendTCPMessage(message, socketFdTCP) == -1) {
    return -1;
  }
  int ret = turnOnSocketTimer(socketFdTCP);
  if (ret == -1) {
    disconnectTCP(serverInfoTCP, socketFdTCP);
    exit(EXIT_FAILURE);
  }
  ret = receiveTCPMessage(responseMessage, args, socketFdTCP);
  if (ret == -1) {
    return -1;
  }
  ret = turnOffSocketTimer(socketFdTCP);
  if (ret == -1) {
    disconnectTCP(serverInfoTCP, socketFdTCP);
    exit(EXIT_FAILURE);
  }
  serverMessage.body = responseMessage;
  return 0;
}

int parseTCPResponse(struct protocolMessage &serverMessage) {
  std::string responseBegin = serverMessage.body;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string status = responseBegin.substr(0, responseBegin.find_first_of(" \n"));
  serverMessage.first = command;
  serverMessage.second = status;
  return handleTCPServerMessage[command](serverMessage);
}

int parseFileArgs(struct fileInfo &info) {
  std::string fileArgs;
  const int ret = receiveTCPMessage(fileArgs, TCP_FILE_ARGS, socketFdTCP);
  if (ret == -1) {
    std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
    return -1;
  }
  info.fileName = fileArgs.substr(0, fileArgs.find_first_of(' '));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info.fileSize = std::stoi(fileArgs.substr(0, fileArgs.find_first_of(' ')));
  fileArgs.erase(0, fileArgs.find_first_of(' '));
  info.delimiter = fileArgs[0];
  if (info.delimiter == ' ') {
    return 0;
  }
  std::cerr << INVALID_FILE_ARGS << std::endl;
  return -1;
}

int generalTCPHandler(std::string message, struct peerInfo peer) {
  struct protocolMessage serverMessage;
  if (createSocketTCP(peer) == -1) {
    return -1;
  }
  if (exchangeTCPMessage(message, serverMessage, TCP_DEFAULT_ARGS) == -1) {
    return -1;
  }
  return parseTCPResponse(serverMessage);
}

int handleRSB(struct protocolMessage response) {
  if (response.second == "OK") {
    struct fileInfo info;
    int ret = parseFileArgs(info);
    if (ret == -1) {
      disconnectTCP(serverInfoTCP, socketFdTCP);
      return -1;
    }
    if (receiveTCPFile(info, SB_DIR, socketFdTCP) == -1) {
      disconnectTCP(serverInfoTCP, socketFdTCP);
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    ret = displayFile(info.fileName, SB_DIR);
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return ret;
  } else if (response.second == "EMPTY") {
    std::cout << SB_FAIL << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return 0;
  }
  disconnectTCP(serverInfoTCP, socketFdTCP);
  return -1;
}

int handleRHL(struct protocolMessage response) {
  if (response.second == "OK") {
    struct fileInfo info;
    const int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
      disconnectTCP(serverInfoTCP, socketFdTCP);
      return -1;
    }
    const int bytesRead = receiveTCPFile(info, H_DIR, socketFdTCP);
    if (bytesRead == -1) {
      disconnectTCP(serverInfoTCP, socketFdTCP);
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << H_SUCCESS(info.fileName, bytesRead) << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return 0;
  } else if (response.second == "NOK") {
    std::cout << H_FAIL << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return 0;
  }
  disconnectTCP(serverInfoTCP, socketFdTCP);
  return -1;
}

int handleRST(struct protocolMessage response) {
  if (response.second == "NOK") {
    std::cout << ST_NOK << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return 0;
  } else if (response.second == "ERR") {
    std::cerr << ST_ERR << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return -1;
  }

  struct fileInfo info;
  const int ret = parseFileArgs(info);
  if (ret == -1) {
    std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return -1;
  }

  const int bytesRead = receiveTCPFile(info, ST_DIR, socketFdTCP);
  if (bytesRead == -1) {
    disconnectTCP(serverInfoTCP, socketFdTCP);
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  if (response.second == "ACT") {
    std::cout << ST_ACT << std::endl;
  } else if (response.second == "FIN") {
    std::cout << ST_FIN << std::endl;
  }

  displayFile(info.fileName, ST_DIR);
  disconnectTCP(serverInfoTCP, socketFdTCP);
  return 0;
}

int sendGSB(struct messageInfo info) {
  if (validateArgsAmount(info.input, SCOREBOARD_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GSB"});
  return generalTCPHandler(message, info.peer);
}

int sendGHL(struct messageInfo info) {
  if (validateArgsAmount(info.input, HINT_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"GHL", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

int sendSTA(struct messageInfo info) {
  if (validateArgsAmount(info.input, STATE_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitStringNewline({"STA", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

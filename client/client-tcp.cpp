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

int disconnectTCP() {
  freeaddrinfo(serverInfoTCP);
  if (close(socketFdTCP) == -1) {
    std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
    return -1;
  }
  return 0;
}

int exchangeTCPMessage(std::string message, struct protocolMessage &serverMessage, int args) {
  if (serverInfoTCP == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::string responseMessage;
  if (sendTCPMessage(message) == -1) {
    return -1;
  }
  int ret = turnOnSocketTimer(socketFdTCP);
  if (ret == -1) {
    disconnectTCP();
    exit(EXIT_FAILURE);
  }
  ret = receiveTCPMessage(responseMessage, args);
  if (ret == -1) {
    return -1;
  }
  ret = turnOffSocketTimer(socketFdTCP);
  if (ret == -1) {
    disconnectTCP();
    exit(EXIT_FAILURE);
  }
  serverMessage.body = responseMessage;
  return 0;
}

int sendTCPMessage(std::string message) {
  size_t bytesSent = 0;
  size_t bytesLeft = message.length();
  size_t msgLen = bytesLeft;
  while (bytesSent < msgLen) {
    ssize_t bytes = send(socketFdTCP, message.c_str() + bytesSent, bytesLeft, 0);
    if (bytes < 0) {
      std::cerr << TCP_SEND_MESSAGE_ERROR << std::endl;
      return -1;
    }
    bytesSent += (size_t)bytes;
    bytesLeft -= (size_t)bytes;
  }
  return (int)bytesSent;
}

int receiveTCPMessage(std::string &message, int args) {
  ssize_t bytesReceived = 0;
  size_t bytesRead = 0;
  int readArgs = 0;
  char c;
  do {
    // FIXME: there will be a problem if the response is "ERR\n"?
    turnOnSocketTimer(socketFdTCP);
    bytesReceived = read(socketFdTCP, &c, 1);
    turnOffSocketTimer(socketFdTCP);
    if (bytesReceived == -1) {
      std::cerr << TCP_RECV_MESSAGE_ERROR << std::endl;
      return -1;
    } else if (c == ' ' || c == '\n') {
      readArgs++;
    }
    message.push_back(c);
    bytesRead += (size_t)bytesReceived;
  } while (bytesReceived != 0 && readArgs < args);
  return (int)bytesRead;
}

int receiveTCPFile(struct fileInfo &info, std::string dir) {
  ssize_t bytesReceived = 0;
  size_t bytesRead = 0;
  size_t bytesLeft = (size_t)info.fileSize;
  // create directory if it doesn't exist
  if (mkdir(dir.c_str(), 0700) == -1 && errno != EEXIST) {
    std::cerr << MKDIR_ERROR(dir) << std::endl;
    exit(EXIT_FAILURE);
  }
  std::fstream file;
  // TODO: create these folders in the client directory
  file.open(dir + "/" + info.fileName, std::ios::out | std::ios::in | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }
  // read from socket and write to file until file size is reached, in chunks
  char buffer[TCP_CHUNK_SIZE];
  do {
    memset(buffer, 0, TCP_CHUNK_SIZE);
    turnOnSocketTimer(socketFdTCP);
    bytesReceived = read(socketFdTCP, buffer, (TCP_CHUNK_SIZE > bytesLeft) ? bytesLeft : TCP_CHUNK_SIZE);
    turnOffSocketTimer(socketFdTCP);
    if (bytesReceived == -1) {
      std::cerr << TCP_RECV_MESSAGE_ERROR << std::endl;
      return -1;
    }
    // print buffer
    file.write(buffer, bytesReceived);
    bytesRead += (size_t)bytesReceived;
    bytesLeft -= (size_t)bytesReceived;
  } while (bytesReceived != 0 && bytesLeft > 0);

  // TODO: should we check if the message ends in a newline?
  file.close();
  return (int)bytesRead;
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
  const int ret = receiveTCPMessage(fileArgs, TCP_FILE_ARGS);
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
  // TODO: check if the last character in body is the expected one
  if (response.second == "OK") {
    struct fileInfo info;
    int ret = parseFileArgs(info);
    if (ret == -1) {
      disconnectTCP();
      return -1;
    }
    if (receiveTCPFile(info, SB_DIR) == -1) {
      disconnectTCP();
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    ret = displayFile(info.fileName, SB_DIR);
    disconnectTCP();
    return ret;
  } else if (response.second == "EMPTY") {
    std::cout << SB_FAIL << std::endl;
    disconnectTCP();
    return 0;
  }
  disconnectTCP();
  return -1;
}

int handleRHL(struct protocolMessage response) {
  if (response.second == "OK") {
    struct fileInfo info;
    const int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
      disconnectTCP();
      return -1;
    }
    const int bytesRead = receiveTCPFile(info, H_DIR);
    if (bytesRead == -1) {
      disconnectTCP();
      return -1;
    }
    std::cout << FILE_RECV_SUCCESS << std::endl;
    std::cout << H_SUCCESS(info.fileName, bytesRead) << std::endl;
    disconnectTCP();
    return 0;
  } else if (response.second == "NOK") {
    std::cout << H_FAIL << std::endl;
    disconnectTCP();
    return 0;
  }
  disconnectTCP();
  return -1;
}

int handleRST(struct protocolMessage response) {
  if (response.second == "NOK") {
    std::cout << ST_NOK << std::endl;
    disconnectTCP();
    return 0;
  } else if (response.second == "ERR") {
    std::cerr << ST_ERR << std::endl;
    disconnectTCP();
    return -1;
  }

  struct fileInfo info;
  const int ret = parseFileArgs(info);
  if (ret == -1) {
    std::cerr << TCP_FILE_ARGS_ERROR << std::endl;
    disconnectTCP();
    return -1;
  }

  const int bytesRead = receiveTCPFile(info, ST_DIR);
  if (bytesRead == -1) {
    disconnectTCP();
    return -1;
  }

  std::cout << FILE_RECV_SUCCESS << std::endl;
  if (response.second == "ACT") {
    std::cout << ST_ACT << std::endl;
  } else if (response.second == "FIN") {
    std::cout << ST_FIN << std::endl;
  }

  displayFile(info.fileName, ST_DIR);
  disconnectTCP();
  return 0;
}

int sendGSB(struct messageInfo info) {
  if (validateArgsAmount(info.input, SCOREBOARD_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GSB"});
  return generalTCPHandler(message, info.peer);
}

int sendGHL(struct messageInfo info) {
  if (validateArgsAmount(info.input, HINT_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GHL", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

int sendSTA(struct messageInfo info) {
  if (validateArgsAmount(info.input, STATE_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"STA", getPlayerID()});
  return generalTCPHandler(message, info.peer);
}

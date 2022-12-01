#include "client-protocol.h"

struct addrinfo *serverInfoTCP;
int socketFdTCP;

// clang-format off
responseHandler handleTCPServerMessage = {
  {"RSB", handleRSB},
  {"RHL", handleRHL},
  {"RST", handleRST}
};
// clang-format on

int createSocketTCP(struct peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer.addr, peer.port, &serverInfoTCP);
  if (connect(socketFdTCP, serverInfoTCP->ai_addr, serverInfoTCP->ai_addrlen) == -1) {
    std::cerr << "[ERR]: Failed to connect to TCP server. Exiting." << std::endl;
    return -1;
  }
  return 0;
}

int disconnectTCP() {
  freeaddrinfo(serverInfoTCP);
  if (close(socketFdTCP) == -1) {
    std::cerr << "[ERR]: Failed to close TCP socket. Exiting." << std::endl;
    return -1;
  }
  return 0;
}

int exchangeTCPMessage(std::string message, struct protocolMessage &serverMessage, int args) {
  if (serverInfoTCP == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;
  std::string responseMessage;

  if (sendTCPMessage(message) == -1) {
    return -1;
  }
  turnOnSocketTimer(socketFdTCP);
  const int ret = receiveTCPMessage(responseMessage, args);
  turnOffSocketTimer(socketFdTCP);
  if (ret == -1) {
    return -1;
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
      std::cerr << "[ERR]: Failed to send message to TCP server." << std::endl;
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
    // FIXME: will there be a problem if the response is "ERR\n"?
    bytesReceived = read(socketFdTCP, &c, 1);
    if (bytesReceived == -1) {
      std::cerr << "[ERR]: Failed to receive message from TCP server." << std::endl;
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
  file.open(dir + "/" + info.fileName, std::fstream::out | std::fstream::binary);
  file.clear();
  if (!file.is_open()) {
    std::cerr << "[ERR]: Failed to open file for writing." << std::endl;
    return -1;
  }
  // read from socket and write to file until file size is reached, in chunks
  char buffer[TCP_CHUNK_SIZE];
  do {
    memset(buffer, 0, TCP_CHUNK_SIZE);
    bytesReceived = read(socketFdTCP, buffer, TCP_CHUNK_SIZE);
    if (bytesReceived == -1) {
      std::cerr << "[ERR]: Failed to receive message from TCP server." << std::endl;
      return -1;
    }
    file.write(buffer, bytesReceived);
    bytesRead += (size_t)bytesReceived;
    bytesLeft -= (size_t)bytesReceived;
  } while (bytesReceived != 0 && bytesLeft > 0);
  // if the last character is a newline, everything is fine, remove it
  file.seekp(-1, std::ios::end);
  if (file.peek() == '\n') {
    file.write("", 1); // clears the newline
    bytesRead--;       // we don't want to count the newline
    file.close();
    return (int)bytesRead;
  }
  std::cerr << "[ERR]: Message does not match expected format." << std::endl;
  file.close();
  return -1;
}

int parseTCPResponse(struct protocolMessage &serverMessage) {
  std::string responseBegin = serverMessage.body;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string status = responseBegin.substr(0, responseBegin.find_first_of(" \n"));
  serverMessage.code = command;
  serverMessage.status = status;
  return handleTCPServerMessage[command](serverMessage);
}

int parseFileArgs(struct fileInfo &info) {
  std::string fileArgs;
  const int ret = receiveTCPMessage(fileArgs, TCP_FILE_ARGS);
  if (ret == -1) {
    return -1;
  }
  info.fileName = fileArgs.substr(0, fileArgs.find_first_of(' '));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info.fileSize = std::stoi(fileArgs.substr(0, fileArgs.find_first_of(' ')));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info.delimiter = fileArgs[0];
  return (info.delimiter == ' ') ? 0 : -1;
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

// TODO: can't forget to close socket
int handleRSB(struct protocolMessage response) {
  // TODO: check if the last character in body is the expected one
  if (response.status == "OK") {
    struct fileInfo info;
    int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cout << "[INFO]: Arguments for file transfer are invalid." << std::endl;
      disconnectTCP();
      return -1;
    }
    if (receiveTCPFile(info, "scoreboard") == -1) {
      disconnectTCP();
      return -1;
    }
    std::cout << "[INFO]: File received successfully." << std::endl;
    std::cout << "SCORE | PLID | WORD | CORRECT GUESSES | TOTAL GUESSES" << std::endl;
    ret = displayFileRank(info.fileName);
    disconnectTCP();
    return ret;
  } else if (response.status == "EMPTY") {
    std::cout << "[INFO]: The server hasn't held any games yet." << std::endl;
    disconnectTCP();
    return 0;
  }
  return -1;
}

int handleRHL(struct protocolMessage response) {
  if (response.status == "OK") {
    struct fileInfo info;
    const int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cout << "[INFO]: Arguments for file transfer are invalid." << std::endl;
      disconnectTCP();
      return -1;
    }
    const int bytesRead = receiveTCPFile(info, "hints");
    if (bytesRead == -1) {
      disconnectTCP();
      return -1;
    }
    std::cout << "[INFO]: File received successfully." << std::endl;
    std::cout << "[HINT]: " << info.fileName << ", " << bytesRead << " bytes." << std::endl;
    disconnectTCP();
    return 0;
  } else if (response.status == "NOK") {
    std::cout << "[INFO]: The server could not send any hints at the moment." << std::endl;
    disconnectTCP();
    return 0;
  }
  return -1;
}

int handleRST(struct protocolMessage response) {
  if (response.status == "NOK") {
    std::cout << "[INFO]: The server could not find any games (neither active nor finished)."
              << std::endl;
    disconnectTCP();
    return 0;
  } else if (response.status == "ERR") {
    std::cout << "[INFO]: The server has encountered an error." << std::endl;
    disconnectTCP();
    return -1;
  }

  struct fileInfo info;
  const int ret = parseFileArgs(info);
  if (ret == -1) {
    std::cout << "[INFO]: Arguments for file transfer are invalid." << std::endl;
    disconnectTCP();
    return -1;
  }

  const int bytesRead = receiveTCPFile(info, "state");
  if (bytesRead == -1) {
    disconnectTCP();
    return -1;
  }

  std::cout << "[INFO]: File received successfully." << std::endl;
  std::cout << "[INFO]: " << info.fileName << ", " << bytesRead << " bytes." << std::endl;

  if (response.status == "ACT") {
    std::cout << "[INFO]: Displaying information about the current game." << std::endl;
  } else if (response.status == "FIN") {
    std::cout << "[INFO]: Displaying information about the most recent finished game." << std::endl;
  }

  displayFile(info.fileName);
  disconnectTCP();
  return 0;
}

int sendGSB(std::string input, struct peerInfo peer) {
  if (validateArgsAmount(input, SCOREBOARD_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GSB"});
  return generalTCPHandler(message, peer);
}

int sendGHL(std::string input, struct peerInfo peer) {
  if (validateArgsAmount(input, HINT_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GHL", plid});
  return generalTCPHandler(message, peer);
}

int sendSTA(std::string input, struct peerInfo peer) {
  if (validateArgsAmount(input, STATE_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GST", plid});
  return generalTCPHandler(message, peer);
}

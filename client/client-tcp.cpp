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

void createSocketTCP(std::string addr, std::string port) {
  socketFdTCP = newSocket(SOCK_STREAM, addr, port, &serverInfoTCP);
  if (connect(socketFdTCP, serverInfoTCP->ai_addr, serverInfoTCP->ai_addrlen) == -1) {
    std::cerr << "[ERR]: Failed to connect to TCP server. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
}

int sendTCPMessage(std::string message) {
  size_t bytesSent = 0;
  size_t bytesLeft = message.length();
  while (bytesSent < message.length()) {
    ssize_t bytes = send(socketFdTCP, message.c_str() + bytesSent, bytesLeft, 0);
    if (bytes == -1) {
      std::cerr << "[ERR]: Failed to send message to TCP server. Exiting." << std::endl;
      exit(EXIT_FAILURE);
    }
    bytesSent += bytes;
    bytesLeft -= bytes;
  }
  return 0;
}

int receiveTCPMessage(std::string *message, int args) {
  size_t bytesReceived = 0;
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
    message->push_back(c);
    bytesRead += bytesReceived;
  } while (bytesReceived != 0 && readArgs < args);
  return (int)bytesRead;
}

int receiveTCPFile(struct fileInfo *info, std::string dir) {
  size_t bytesReceived = 0;
  size_t bytesRead = 0;
  size_t bytesLeft = (size_t)info->fileSize;
  // create directory if it doesn't exist
  if (mkdir(dir.c_str(), 0700) == -1 && errno != EEXIST) {
    std::cerr << MKDIR_ERROR(dir) << std::endl;
    exit(EXIT_FAILURE);
  }
  std::fstream file;
  file.open(dir + "/" + info->fileName, std::fstream::out | std::fstream::binary);
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
    bytesRead += bytesReceived;
    bytesLeft -= bytesReceived;
  } while (bytesReceived != 0 && bytesLeft > 0);
  // if the last character is a newline, everything is fine, remove it
  file.seekp(-1, std::ios::end);
  if (file.peek() == '\n') {
    file.trunc;
    bytesRead--; // we don't want to count the newline
  } else {
    std::cerr << "[ERR]: Message does not match expected format." << std::endl;
    return -1;
  }
  file.close();
  return (int)bytesRead;
}

int exchangeTCPMessage(std::string message, struct protocolMessage *serverMessage, int args) {
  if (serverInfoTCP == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;
  std::string *responseMessage = new std::string();

  sendTCPMessage(message);
  turnOnSocketTimer(socketFdTCP);
  receiveTCPMessage(responseMessage, args);
  turnOffSocketTimer(socketFdTCP);
  serverMessage->body = *responseMessage;
  return 0;
}

int parseTCPResponse(struct protocolMessage *serverMessage) {
  std::string responseBegin = serverMessage->body;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string status = responseBegin.substr(0, responseBegin.find_first_of(' \n'));
  serverMessage->code = command;
  serverMessage->status = status;
  return handleTCPServerMessage[command](*serverMessage);
}

int generalTCPHandler(std::string message) {
  struct protocolMessage *serverMessage;
  if (exchangeTCPMessage(message, serverMessage, TCP_DEFAULT_ARGS) == -1) {
    return -1;
  }
  return parseTCPResponse(serverMessage);
}

int parseFileArgs(struct fileInfo *info) {
  std::string fileArgs;
  const int ret = receiveTCPMessage(&fileArgs, TCP_FILE_ARGS);
  if (ret == -1) {
    return -1;
  }
  info->fileName = fileArgs.substr(0, fileArgs.find_first_of(' '));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info->fileSize = std::stoi(fileArgs.substr(0, fileArgs.find_first_of(' ')));
  fileArgs.erase(0, fileArgs.find_first_of(' ') + 1);
  info->delimiter = fileArgs[0];
  return (info->delimiter == ' ') ? 0 : -1;
}

// TODO: can't forget to close socket
int handleRSB(struct protocolMessage response) {
  // TODO: check if the last character in body is the expected one
  if (response.status == "OK") {
    struct fileInfo *info;
    const int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cout << "[INFO]: Arguments for file transfer are invalid." << std::endl;
      // TODO: close TCP socket
      return -1;
    }
    if (receiveTCPFile(info, "scoreboard") == -1) {
      // TODO: close TCP socket
      return -1;
    }
    std::cout << "[INFO]: File received successfully." << std::endl;
    std::cout << "SCORE | PLID | WORD | CORRECT GUESSES | TOTAL GUESSES" << std::endl;
    return displayFile(info->fileName);
    // TODO: close TCP socket
  } else if (response.status == "EMPTY") {
    std::cout << "[INFO]: The server hasn't held any games yet." << std::endl;
    // TODO: close TCP socket
    return 0;
  }
  return -1;
}

int handleRHL(struct protocolMessage response) {
  if (response.status == "OK") {
    struct fileInfo *info;
    const int ret = parseFileArgs(info);
    if (ret == -1) {
      std::cout << "[INFO]: Arguments for file transfer are invalid." << std::endl;
      // TODO: close TCP socket
      return -1;
    }
    if (receiveTCPFile(info, "hints") == -1) {
      // TODO: close TCP socket
      return -1;
    }
    std::cout << "[INFO]: File received successfully, stored in the hints directory." << std::endl;
  } else if (response.status == "NOK") {
    std::cout << "[INFO]: The server could not send any hints at the moment." << std::endl;
    // TODO: close TCP socket
  }
  return -1;
}

int handleRST(struct protocolMessage response) {
  if (response.status == "ACT") {
    std::cout << "[INFO]: The server has the following active game:" << std::endl;
  } else if (response.status == "FIN") {
    std::cout << "[INFO]: The server has the following finished games:" << std::endl;
  } else if (response.status == "NOK") {
    std::cout << "[INFO]: The server could not find any games for the given player." << std::endl;
    // TODO: close TCP socket
  }
}

int sendGSB(std::string input) {
  if (validateArgsAmount(input, SCOREBOARD_ARGS) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GSB"});
  return generalTCPHandler(message);
}

int sendGHL(std::string input) {
  if (validateArgsAmount(input, HINT_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GHL", plid});
  return generalTCPHandler(message);
}

int sendSTA(std::string input) {
  if (validateArgsAmount(input, STATE_ARGS) == -1) {
    return -1;
  }
  const size_t pos1 = input.find(' ');
  const std::string plid = input.substr(pos1 + 1);
  if (validatePlayerID(plid) == -1) {
    return -1;
  }
  const std::string message = buildSplitString({"GST", plid});
  return generalTCPHandler(message);
}

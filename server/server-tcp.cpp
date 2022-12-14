#include "server-protocol.h"

// TCP related socket variables
struct addrinfo hintsTCP, *resTCP;
int socketFdTCP, newConnectionFd;
socklen_t addrlenTCP;
bool verboseTCP;
char hostTCP[NI_MAXHOST], serviceTCP[NI_MAXSERV]; // consts in <netdb.h>
char bufferTCP[TCP_CHUNK_SIZE];
pid_t pid;
struct sigaction actTCP;

// clang-format off
responseHandler handleTCPClientMessage = {
  {"GSB", handleGSB},
  {"GHL", handleGHL},
  {"STA", handleSTA}
};
// clang-format on

int setServerTCPParameters(bool vParam) {
  verboseTCP = vParam;
  return 0;
}

int createSocketTCP(struct peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer, &hintsTCP, &resTCP);
  if (socketFdTCP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (listen(socketFdTCP, MAX_TCP_CONNECTION_REQUESTS) == -1) {
    std::cerr << TCP_LISTEN_ERROR << std::endl;
    exit(EXIT_FAILURE); // TODO: exit gracefully here
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  // Ignore SIGCHLD to avoid zombie processes
  memset(&actTCP, 0, sizeof(actTCP));
  actTCP.sa_handler = SIG_IGN;
  if (sigaction(SIGCHLD, &actTCP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    exit(EXIT_FAILURE); // TODO: exit gracefully here
  }

  // Igonre SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actTCP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    exit(EXIT_FAILURE); // TODO: exit gracefully here
  }

  return socketFdTCP;
}

int disconnectTCP() { return disconnectSocket(resTCP, socketFdTCP); }

int parseTCPMessage(std::string request) {
  std::string responseBegin = request;
  struct protocolMessage serverMessage;
  const std::string command = responseBegin.substr(0, 3);
  responseBegin.erase(0, 4);
  const std::string plid = responseBegin.substr(0, responseBegin.find_first_of(" \n"));
  serverMessage.first = command;
  serverMessage.second = plid;
  serverMessage.body = request;
  return handleTCPClientMessage[command](serverMessage);
}

int generalTCPHandler(struct peerInfo peer) {
  struct protocolMessage response;
  if (createSocketTCP(peer) == -1) {
    return -1;
  }
  while (true) {
    addrlenTCP = sizeof(peer.addr);
    if ((newConnectionFd = accept(socketFdTCP, (struct sockaddr *)&peer.addr, &addrlenTCP)) == -1) {
      std::cerr << TCP_ACCEPT_ERROR << std::endl;
      exit(EXIT_FAILURE); // TODO: exit gracefully here
    }

    if ((pid = fork()) == -1) {
      std::cerr << FORK_ERROR << std::endl;
      exit(EXIT_FAILURE); // TODO: exit gracefully here
    }

    if (pid != 0) { // Father process
      close(newConnectionFd);
    } else { // Child process
      close(socketFdTCP);
      if (read(newConnectionFd, bufferTCP, TCP_CHUNK_SIZE) == -1) {
        std::cerr << TCP_READ_ERROR << std::endl;
        exit(EXIT_FAILURE); // TODO: exit gracefully here
      }

      std::cout << "[INFO]: Received message: " << bufferTCP;
      if (verboseTCP) {
        int errcode = getnameinfo((struct sockaddr *)&peer.addr, addrlenTCP, hostTCP, sizeof hostTCP,
                                  serviceTCP, sizeof serviceTCP, 0);
        if (errcode != 0)
          std::cerr << VERBOSE_ERROR(errcode) << std::endl;
        else {
          std::cout << VERBOSE_SUCCESS("TCP", hostTCP, serviceTCP) << std::endl;
        }
      }
      parseTCPMessage(std::string(bufferTCP));
      exit(EXIT_SUCCESS);
    }
  }
}

// Server message handlers
int handleGSB(struct protocolMessage message) {
  std::cout << "[INFO]: Received GSB message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    return sendTCPMessage(buildSplitStringNewline({"ERR"}), newConnectionFd);
  }

  std::string response;
  int ret = getScoreboard(response);
  switch (ret) {
    case SCOREBOARD_ERROR:
      // TODO: This doesn't exist
      response = buildSplitStringNewline({"RSB", "NOK"});
      break;
    case SCOREBOARD_EMPTY:
      response = buildSplitStringNewline({"RSB", "EMPTY"});
      break;
    case SCOREBOARD_SUCCESS:
      response = buildSplitString({"RSB", "OK", response});
      return sendTCPFile(response.append(" "), newConnectionFd, SCORES_PATH);
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RSB", "ERR"});
      break;
  }
  return sendTCPMessage(response, newConnectionFd);
}

int handleGHL(struct protocolMessage message) {
  std::cout << "[INFO]: Received GHL message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"ERR"});
    return sendTCPMessage(response, newConnectionFd);
  }

  const std::string plid = message.second;
  std::string file;
  std::string response;

  int ret = getHint(plid, response, file);
  switch (ret) {
    case HINT_ERROR:
      response = buildSplitStringNewline({"RHL", "NOK"});
      break;
    case HINT_SUCCESS:
      response = buildSplitString({"RHL", "OK", response});
      return sendTCPFile(response.append(" "), newConnectionFd, file);
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RHL", "ERR"});
      break;
  }

  return sendTCPMessage(response, newConnectionFd);
}

int handleSTA(struct protocolMessage message) {
  std::cout << "[INFO]: Received STA message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"ERR"});
    return sendTCPMessage(response, newConnectionFd);
  }

  const std::string plid = message.second;
  std::string file;
  std::string response;

  int ret = getState(plid, response, file);
  switch (ret) {
    case STATE_ERROR:
      response = buildSplitStringNewline({"RST", "NOK"});
      return sendTCPMessage(response, newConnectionFd);
    case STATE_ONGOING:
      response = buildSplitString({"RST", "ACT", response});
      break;
    case STATE_FINISHED:
      response = buildSplitString({"RST", "FIN", response});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RST", "ERR"});
      return sendTCPMessage(response, newConnectionFd);
  }

  ret = sendTCPFile(response.append(" "), newConnectionFd, file);

  // remove tmp file
  std::string tmpFile = TMP_PATH(plid);
  if (remove(tmpFile.c_str()) != 0) {
    std::cerr << "[ERR]: Error deleting file" << std::endl;
  } else {
    std::cout << "[INFO]: Deleted temporary file " << tmpFile << std::endl;
  }
  return ret;
}

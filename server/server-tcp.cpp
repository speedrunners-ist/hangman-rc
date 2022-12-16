#include "server-protocol.h"

// TCP related socket variables
struct addrinfo hintsTCP, *resTCP;
int socketFdTCP, newConnectionFd;
bool verboseTCP;
char hostTCP[NI_MAXHOST], serviceTCP[NI_MAXSERV]; // consts in <netdb.h>
char bufferTCP[TCP_CHUNK_SIZE];
struct sigaction actTCP;

void signalHandlerTCP(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectTCP();
  std::cout << EXIT_PROGRAM << std::endl;
  exit(signum);
}

void signalHandlerTCPchild(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectTCPchild();
  exit(signum);
}

// clang-format off
responseHandler handleTCPClientMessage = {
  {"GSB", handleGSB},
  {"GHL", handleGHL},
  {"STA", handleSTA}
};
// clang-format on

void setServerTCPParameters(bool vParam) { verboseTCP = vParam; }

int createSocketTCP(peerInfo peer) {
  socketFdTCP = newSocket(SOCK_STREAM, peer, &hintsTCP, &resTCP);
  if (socketFdTCP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (listen(socketFdTCP, MAX_TCP_CONNECTION_REQUESTS) == -1) {
    std::cerr << TCP_LISTEN_ERROR << std::endl;
    return -1;
  }

  signal(SIGINT, signalHandlerTCP);
  signal(SIGTERM, signalHandlerTCP);

  // Ignore SIGCHLD to avoid zombie processes
  memset(&actTCP, 0, sizeof(actTCP));
  actTCP.sa_handler = SIG_IGN;
  if (sigaction(SIGCHLD, &actTCP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    return -1;
  }

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actTCP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    return -1;
  }

  return socketFdTCP;
}

int disconnectTCP() { return disconnectSocket(resTCP, socketFdTCP); }
int disconnectTCPchild() { return disconnectSocket(resTCP, newConnectionFd); }

int generalTCPHandler(peerInfo peer) {
  protocolMessage message;
  pid_t pid;
  if (createSocketTCP(peer) == -1) {
    return -1;
  }

  // Listening for incoming connections
  while (true) {
    newConnectionFd = accept(socketFdTCP, resTCP->ai_addr, &resTCP->ai_addrlen);
    if (newConnectionFd == -1) {
      std::cerr << TCP_ACCEPT_ERROR << std::endl;
      return -1;
    }

    pid = fork();
    if (pid == -1) {
      std::cerr << FORK_ERROR << std::endl;
      return -1;
    }

    if (pid == 0) { // Child process
      signal(SIGINT, signalHandlerTCPchild);
      if (close(socketFdTCP) == -1) {
        std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
        return -1;
      } else if (turnOnSocketTimer(newConnectionFd) == -1) {
        return -1;
      }

      std::cout << "[INFO]: New connection" << std::endl;
      if (read(newConnectionFd, bufferTCP, TCP_CHUNK_SIZE) == -1) {
        std::cerr << TCP_READ_ERROR << std::endl;
        return -1;
      }

      if (turnOffSocketTimer(newConnectionFd) == -1) {
        return -1;
      }
      std::cout << "[INFO]: Received message: " << bufferTCP;
      if (verboseTCP) {
        displayVerbose(peer, hostTCP, serviceTCP, "TCP");
      }

      if (parseTCPMessage(std::string(bufferTCP), message) == -1) {
        std::cerr << PARSE_ERROR << std::endl;
        sendTCPMessage(buildSplitStringNewline({"ERR"}), resTCP, newConnectionFd);
        continue;
      }

      messageTCPHandler(newConnectionFd, resTCP, message, handleTCPClientMessage);
      std::cout << "[INFO]: Closing connection" << std::endl;
      if (close(newConnectionFd) == -1) {
        std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
        return -1;
      }
      exit(EXIT_SUCCESS);
    }

    if (close(newConnectionFd) == -1) {
      std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
      return -1;
    }
  }
}

// Server message handlers
int handleGSB(protocolMessage message) {
  std::cout << "[INFO]: Received GSB message" << std::endl;

  if (message.body.compare("GSB\n")) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"ERR"});
    return sendTCPMessage(response, resTCP, newConnectionFd);
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
      return sendTCPFile(response.append(" "), resTCP, newConnectionFd, SCORES_PATH);
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RSB", "ERR"});
      break;
  }
  return sendTCPMessage(response, resTCP, newConnectionFd);
}

int handleGHL(protocolMessage message) {
  std::cout << "[INFO]: Received GHL message" << std::endl;

  const std::string plid = message.second;
  if (!validPlayerID(plid)) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"ERR"});
    return sendTCPMessage(response, resTCP, newConnectionFd);
  }

  std::string file;
  std::string response;

  const int ret = getHint(plid, response, file);
  const std::string fileName = std::filesystem::path(file).filename();
  switch (ret) {
    case HINT_ERROR:
      response = buildSplitStringNewline({"RHL", "NOK"});
      break;
    case HINT_SUCCESS:
      appendGameFile(plid, HINT, fileName);
      response = buildSplitString({"RHL", "OK", response});
      return sendTCPFile(response.append(" "), resTCP, newConnectionFd, file);
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RHL", "ERR"});
      break;
  }

  return sendTCPMessage(response, resTCP, newConnectionFd);
}

int handleSTA(protocolMessage message) {
  std::cout << "[INFO]: Received STA message" << std::endl;
  const std::string plid = message.second;
  if (!validPlayerID(plid)) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"RST", "NOK"});
    return sendTCPMessage(response, resTCP, newConnectionFd);
  }

  std::string file;
  std::string response;

  int ret = getState(plid, response, file);
  switch (ret) {
    case STATE_ERROR:
      response = buildSplitStringNewline({"RST", "NOK"});
      return sendTCPMessage(response, resTCP, newConnectionFd);
    case STATE_ONGOING:
      response = buildSplitString({"RST", "ACT", response});
      break;
    case STATE_FINISHED:
      response = buildSplitString({"RST", "FIN", response});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RST", "ERR"});
      return sendTCPMessage(response, resTCP, newConnectionFd);
  }

  ret = sendTCPFile(response.append(" "), resTCP, newConnectionFd, file);

  // remove tmp file
  std::string tmpFile = TMP_PATH(plid);
  if (remove(tmpFile.c_str()) != 0) {
    std::cerr << "[ERR]: Error deleting file" << std::endl;
  } else {
    std::cout << "[INFO]: Deleted temporary file " << tmpFile << std::endl;
  }
  return ret;
}

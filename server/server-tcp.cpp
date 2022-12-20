#include "server-protocol.h"

int childConnectionFd;
char bufferTCP[TCP_CHUNK_SIZE];

void signalHandlerTCP(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectSocket(getResTCP(), getSocketFdTCP());
  std::cout << EXIT_PROGRAM << std::endl;
  exit(signum);
}

void signalHandlerTCPchild(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectSocket(getResTCP(), childConnectionFd);
  exit(signum);
}

// clang-format off
responseHandler handleTCPClientMessage = {
  {"GSB", handleGSB},
  {"GHL", handleGHL},
  {"STA", handleSTA}
};
// clang-format on

int generalTCPHandler(peerInfo peer) {
  const bool verbose = checkVerbose();
  protocolMessage request;
  pid_t pid;
  if (createSocket(SOCK_STREAM, peer, signalHandlerTCP) == -1) {
    return -1;
  }

  // Listening for incoming connections
  while (true) {
    childConnectionFd = accept(getSocketFdTCP(), getResTCP()->ai_addr, &getResTCP()->ai_addrlen);
    if (childConnectionFd == -1) {
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
      if (close(getSocketFdTCP()) == -1) {
        std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
        return -1;
      } else if (turnOnSocketTimer(childConnectionFd) == -1) {
        return -1;
      }

      if (read(childConnectionFd, bufferTCP, TCP_CHUNK_SIZE) == -1) {
        std::cerr << TCP_READ_ERROR << std::endl;
        return -1;
      }

      if (turnOffSocketTimer(childConnectionFd) == -1) {
        return -1;
      }

      if (verbose) {
        displayPeerInfo(getResTCP(), "TCP");
      }

      if (parseMessage(std::string(bufferTCP), request) == -1) {
        std::cerr << PARSE_ERROR << std::endl;
        sendTCPMessage(ERR, getResTCP(), childConnectionFd);
        continue;
      }

      if (verbose) {
        std::cout << "[INFO]: Received the following message: " << request.body;
      }
      messageTCPHandler(request, handleTCPClientMessage, childConnectionFd, getResTCP());
      if (close(childConnectionFd) == -1) {
        std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
        return -1;
      }
      exit(EXIT_SUCCESS);
    }

    if (close(childConnectionFd) == -1) {
      std::cerr << TCP_SOCKET_CLOSE_ERROR << std::endl;
      return -1;
    }
  }
}

// Server message handlers
int handleGSB(protocolMessage message) {
  if (!validArgsAmount(message.body, GSB_ARGS)) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = ERR;
    return sendTCPMessage(response, getResTCP(), childConnectionFd);
  }

  std::string response;
  int ret = getScoreboard(response);
  switch (ret) {
    case SCOREBOARD_EMPTY:
      response = buildSplitStringNewline({"RSB", "EMPTY"});
      return sendTCPMessage(response, getResTCP(), childConnectionFd);
    case SCOREBOARD_SUCCESS:
    default:
      response = buildSplitString({"RSB", "OK", response});
      return sendTCPFile(response.append(" "), getResTCP(), childConnectionFd, SCORES_PATH);
  }

  std::cerr << INTERNAL_ERROR << std::endl;
  return sendTCPMessage(ERR, getResTCP(), childConnectionFd);
}

int handleGHL(protocolMessage message) {
  const std::string plid = message.status;
  if (!validArgsAmount(message.body, GHL_ARGS) || !validPlayerID(plid)) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = ERR;
    return sendTCPMessage(response, getResTCP(), childConnectionFd);
  }

  std::string file;
  std::string response;
  const int ret = getHint(plid, response, file);
  const std::string fileName = std::filesystem::path(file).filename();
  switch (ret) {
    case HINT_NOK:
      response = buildSplitStringNewline({"RHL", "NOK"});
      return sendTCPMessage(response, getResTCP(), childConnectionFd);
    case HINT_SUCCESS:
    default:
      appendGameFile(plid, HINT, fileName);
      response = buildSplitString({"RHL", "OK", response});
      return sendTCPFile(response.append(" "), getResTCP(), childConnectionFd, file);
  }

  std::cerr << INTERNAL_ERROR << std::endl;
  response = ERR;
  return sendTCPMessage(response, getResTCP(), childConnectionFd);
}

int handleSTA(protocolMessage message) {
  const std::string plid = message.status;
  if (!validArgsAmount(message.body, STA_ARGS) || !validPlayerID(plid)) {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"RST", "NOK"});
    return sendTCPMessage(response, getResTCP(), childConnectionFd);
  }

  std::string file;
  std::string response;
  int ret = getState(plid, response, file);
  const int sta = ret;
  switch (ret) {
    case STATE_NOK:
      response = buildSplitStringNewline({"RST", "NOK"});
      return sendTCPMessage(response, getResTCP(), childConnectionFd);
    case STATE_ONGOING:
      response = buildSplitString({"RST", "ACT", response});
      break;
    case STATE_FINISHED:
    default:
      response = buildSplitString({"RST", "FIN", response});
  }

  ret = sendTCPFile(response.append(" "), getResTCP(), childConnectionFd, file);

  std::string tmpFile = TMP_PATH(plid);
  if (sta != STATE_FINISHED && !std::filesystem::remove(tmpFile)) {
    std::cerr << DELETE_FILE_ERROR(tmpFile) << std::endl;
    return -1;
  }

  std::cout << DELETE_FILE_SUCCESS(tmpFile) << std::endl;
  return ret;
}

#include "server-protocol.h"

static struct addrinfo hintsTCP, *resTCP;
static int socketFdTCP, newfd;
static socklen_t addrlen;
static ssize_t n, nw;
static char *ptr, buffer[128];
static bool verbose;
static char host[NI_MAXHOST], service[NI_MAXSERV]; // consts in <netdb.h>
static peerInfo dummy_peer;

// clang-format off
static responseHandler handleTCPClientMessage = {
  {"GSB", handleGSB},
  {"GHL", handleGHL},
  {"STA", handleSTA}
};
// clang-format on

int setServerTCPParameters(bool vParam) {
  verbose = vParam;
  return 0;
}

int serverSENDTCPMesage(std::string message, std::string filePath) { return 0; }

void createSocketTCP(std::string addr, std::string port) {
  socketFdTCP = newSocket(SOCK_STREAM, addr, port, &hintsTCP, &resTCP);
  if (socketFdTCP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  if (listen(socketFdTCP, 5) == -1) /*error*/
    exit(1);

  // TODO: fix loop
  while (true) {
    addrlen = sizeof(addr);

    // TODO: fix accept
    if ((newfd = accept(socketFdTCP, (struct sockaddr *)&addr, &addrlen)) == -1)
      /*error*/ exit(1);

    n = read(newfd, buffer, 128);
    if (n == -1)
      /*error*/ exit(1);
    std::cout << "[INFO]: Received message: " << buffer;
    int errcode = getnameinfo(resTCP->ai_addr, addrlen, host, sizeof host, service, sizeof service, 0);
    if (verbose) {
      std::cerr << VERBOSE_ERROR(errcode) << std::endl;
    } else {
      std::cout << VERBOSE_SUCCESS(host, service) << std::endl;
    }

    // generalTCPHandler(std::string(buffer), dummy_peer);
    memset(buffer, 0, 128);
    close(newfd);
  }
}
// Server message handlers
int handleGSB(struct protocolMessage message) {
  std::cout << "[INFO]: Received GSB message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    return serverSENDTCPMesage(buildSplitStringNewline({"ERR"}), "");
  }

  std::string file = "";
  std::string response;

  int ret = getScoreboard(response);
  switch (ret) {
    case SCOREBOARD_EMPTY:
      response = buildSplitStringNewline({"RSG", "EMPTY"});
      break;
    case SCOREBOARD_SUCCESS:
      response = buildSplitStringNewline({"RSG", "OK", response});
      file = SCORES_PATH;
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RSG", "ERR"});
      break;
  }

  return serverSENDTCPMesage(response, file);
}

int handleGHL(struct protocolMessage message) {
  std::cout << "[INFO]: Received GHL message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    std::string response = buildSplitStringNewline({"ERR"});
    return serverSENDTCPMesage(response, "");
  }

  const std::string plid = message.second;
  std::string file = "";
  std::string response;

  int ret = getHint(plid, response, file);
  switch (ret) {
    case HINT_ERROR:
      response = buildSplitStringNewline({"RHL", "NOK"});
      break;
    case HINT_SUCCESS:
      response = buildSplitStringNewline({"RHL", "OK", response});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RHL", "ERR"});
      break;
  }

  return serverSENDTCPMesage(response, file);
}

int handleSTA(struct protocolMessage message) {
  std::cout << "[INFO]: Received STA message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    return serverSENDTCPMesage(buildSplitStringNewline({"ERR"}), "");
  }

  const std::string plid = message.second;
  std::string file = "";
  std::string response;

  int ret = getState(plid, response, file);
  switch (ret) {
    case STATE_ERROR:
      response = buildSplitStringNewline({"RST", "NOK"});
      break;
    case STATE_ONGOING:
      response = buildSplitStringNewline({"RST", "ACT", response});
      break;
    case STATE_FINISHED:
      response = buildSplitStringNewline({"RST", "FIN", response});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RST", "ERR"});
      break;
  }

  return serverSENDTCPMesage(response, file);
}

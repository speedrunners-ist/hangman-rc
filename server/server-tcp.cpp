#include "server-protocol.h"

static struct addrinfo hintsTCP, *resTCP;
static int socketFdTCP, newfd;
static socklen_t addrlen;
static ssize_t n;
static bool verbose;
static char host[NI_MAXHOST], service[NI_MAXSERV]; // consts in <netdb.h>
static peerInfo dummy_peer;
static char buffer[TCP_CHUNK_SIZE];

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

int serverSENDTCPMesage(std::string message, std::string filePath) {
  if (write(newfd, message.c_str(), message.size()) == -1) {
    std::cerr << TCP_SEND_MESSAGE_ERROR << std::endl;
    return -1;
  }
  std::cout << "[INFO]: Sent message: " << message << "|" << std::endl;

  std::cout << "[INFO]: Sending file: " << filePath << std::endl;
  if (filePath == "")
    return 0;

  std::ifstream file(filePath, std::ios::binary);

  if (!file.is_open()) {
    std::cerr << TCP_FILE_SEND_ERROR << std::endl;
    return -1;
  }

  write(newfd, " ", 1);

  while (file) {
    file.read(buffer, TCP_CHUNK_SIZE);

    ssize_t bytes_read = file.gcount();
    if (bytes_read == 0)
      break;

    if (write(newfd, buffer, TCP_CHUNK_SIZE) == -1) {
      std::cerr << TCP_FILE_SEND_ERROR << std::endl;
      return -1;
    }

    memset(buffer, 0, TCP_CHUNK_SIZE);
  }

  return 0;
}

int generalTCPHandler(std::string request) {
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

void createSocketTCP(std::string addr, std::string port) {
  socketFdTCP = newSocket(SOCK_STREAM, addr, port, &hintsTCP, &resTCP);

  std::cout << socketFdTCP << std::endl;

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
    int errcode =
        getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, service, sizeof service, 0);
    if (verbose) {
      if (errcode != 0)
        std::cerr << VERBOSE_ERROR(errcode) << std::endl;
      else {
        std::cout << VERBOSE_SUCCESS(host, service) << std::endl;
      }
    }

    generalTCPHandler(std::string(buffer));
    memset(buffer, 0, 128);
  }
}
// Server message handlers
int handleGSB(struct protocolMessage message) {
  if (message.body.back() != '\n') {
    std::cerr << TCP_RESPONSE_ERROR << std::endl;
    return serverSENDTCPMesage(buildSplitStringNewline({"ERR"}), "");
  }

  std::string file = "";
  std::string response;

  int ret = getScoreboard(response);
  switch (ret) {
    case SCOREBOARD_EMPTY:
      response = buildSplitStringNewline({"RSB", "EMPTY"});
      break;
    case SCOREBOARD_SUCCESS:
      response = buildSplitString({"RSB", "OK", response});
      file = SCORES_PATH;
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RSB", "ERR"});
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
      response = buildSplitString({"RHL", "OK", response});
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
      response = buildSplitString({"RST", "ACT", response});
      break;
    case STATE_FINISHED:
      response = buildSplitString({"RST", "FIN", response});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RST", "ERR"});
      break;
  }

  std::cout << "[INFO]: Sending response: " << response << std::endl;
  std::cout << "[INFO]: Sending file: " << file << std::endl;

  serverSENDTCPMesage(response, file);
  // clearTmpFile(file);

  return 0;
}

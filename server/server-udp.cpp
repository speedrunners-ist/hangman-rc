#include "server-protocol.h"

static struct addrinfo *serverInfo;
static int socketFd;
static char responseUDP[UDP_RECV_SIZE];
static socklen_t addrlen;
static struct sockaddr_in addrClient;
static char buffer[UDP_RECV_SIZE];
static ssize_t nread;
struct addrinfo hints;

// clang-format off
static commandHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

void createSocketUDP(std::string addr, std::string port) {
  socketFd = newSocket(SOCK_DGRAM, addr, port, &hints, &serverInfo);

  // Listen for incoming connections
  while (1) {

    addrlen = sizeof(addrClient);
    if (recvfrom(socketFd, buffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addrClient, &addrlen) ==
        -1) /*error*/
      exit(1);

    parseUDPResponse(buffer);

    std::cout << "[INFO]: Received message: " << buffer << std::endl;
  }
}

int exchangeUDPMessage(std::string message, char *response) {
  if (serverInfo == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message;

  int triesLeft = UDP_TRIES;

  if (sendto(socketFd, message.c_str(), message.length(), 0, serverInfo->ai_addr,
             serverInfo->ai_addrlen) == -1) {
    std::cerr << SENDTO_ERROR << std::endl;
    return -1;
  } else {
    return 0;
  }

  std::cerr << RECVFROM_ERROR << std::endl;
  return -1;
}

int parseUDPResponse(char *response) {
  const std::string responseStr(response);
  const size_t pos1 = responseStr.find(' ');

  if (pos1 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string code = responseStr.substr(0, pos1);
  // TODO: SEe this
  const bool lookingForEndLine = (code == "RQT" || code == "RRV");
  const char lookupStatusChar = '\n';
  const size_t pos2 = responseStr.find(lookupStatusChar, pos1 + 1);
  if (lookingForEndLine && pos2 != std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  } else if (!lookingForEndLine && pos2 == std::string::npos) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }
  const std::string status = responseStr.substr(pos1 + 1, pos2 - pos1 - 1);
  const struct protocolMessage serverResponse = {code, pos1, status, pos2, responseStr};
  return handleUDPClientMessage[code](serverResponse);
}

// UDP handlers
int generalUDPHandler(std::string message) {
  memset(responseUDP, 0, UDP_RECV_SIZE);
  const int ret = exchangeUDPMessage(message, responseUDP);
  if (ret == -1) {
    return -1;
  }
  // return parseUDPResponse(responseUDP);
}

// UDP server message Send
int sendRSG(std::string input) { return 0; }
int sendRLG(std::string input) { return 0; }
int sendRWG(std::string input) { return 0; }
int sendRQT(std::string input) { return 0; }
int sendRRV(std::string input) { return 0; }

// Server message handlers
int handleSNG(struct protocolMessage message) {

  std::string body = message.body;

  const size_t pos1 = body.find(' ');

  std::string plid = body.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  if (validatePlayerID(plid) == 0) {
    setPlayerID(plid);
    const std::string response = "RSG OK ";
    return generalUDPHandler(response);
  }
  return -1;
}
int handlePLG(struct protocolMessage message) {
  sendRLG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handlePWG(struct protocolMessage message) {
  sendRWG(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleQUT(struct protocolMessage message) {
  sendRQT(message.code);
  std::cout << message.code << std::endl;
  return 0;
}
int handleREV(struct protocolMessage message) {
  sendRRV(message.code);
  std::cout << message.code << std::endl;
  return 0;
}

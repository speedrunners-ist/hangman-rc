#include "server-protocol.h"

static struct addrinfo *serverInfo;
static int socketFd;
static char responseUDP[UDP_RECV_SIZE];
static socklen_t addrlen;
static struct sockaddr_in addrClient;
static char buffer[UDP_RECV_SIZE];
static ssize_t nread;
struct addrinfo hints;
static bool verbose;
static char host[NI_MAXHOST], service[NI_MAXSERV]; // consts in <netdb.h>

// clang-format off
static commandHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

void setServerParamaters(std::string filepath, bool verboseValue) {
  verbose = verboseValue;
  setPath(filepath);
}

void createSocketUDP(std::string addr, std::string port) {
  int errcode;

  socketFd = newSocket(SOCK_DGRAM, addr, port, &hints, &serverInfo);

  // Listen for incoming connections
  while (1) {

    addrlen = sizeof(addrClient);
    if (recvfrom(socketFd, buffer, UDP_RECV_SIZE, 0, (struct sockaddr *)&addrClient, &addrlen) ==
        -1) /*error*/
      exit(1);
    std::cout << "[INFO]: Received message: " << buffer << std::endl;

    // TODO put type of request
    if (verbose) {
      if ((errcode = getnameinfo((struct sockaddr *)&addrClient, addrlen, host, sizeof host,
                                 service, sizeof service, 0)) != 0)
        fprintf(stderr, "error: getnameinfo: %s\n", gai_strerror(errcode));
      else
        printf("Message sent by [%s:%s]\n", host, service);
    }

    parseUDPResponse(buffer);
  }
}

int exchangeUDPMessage(std::string message, char *response) {
  if (serverInfo == NULL) {
    std::cerr << GETADDRINFO_ERROR << std::endl;
    return -1;
  }

  std::cout << "[INFO]: Sending message: " << message << std::endl;

  if (sendto(socketFd, message.c_str(), message.length(), 0, (struct sockaddr *)&addrClient,
             addrlen) == -1) {
    std::cerr << SENDTO_ERROR << std::endl;
    return -1;
  } else {
    return 0;
  }

  std::cerr << SENDTO_ERROR << std::endl;
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

  // TODO: Check if player is already in the game
  if (validatePlayerID(plid) == 0) {
    setPlayerID(plid);

    std::string wordDescription = readWordFromFile();

    std::cout << "[INFO]: Sending message: " << wordDescription << std::endl;

    const std::string response = "RSG OK " + wordDescription;

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

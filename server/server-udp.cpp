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

    memset(buffer, 0, UDP_RECV_SIZE);
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
int sendRSG(std::string input) {
  std::string plid = input;

  std::string arguments;
  int ret = createGameSession(plid, arguments);
  std::string response;

  switch (ret) {
    case CREATE_GAME_ERROR:
      response = buildSplitString({"RSG", "NOK"});
      break;
    case CREATE_GAME_SUCCESS:
      response = buildSplitString({"RSG", "OK", arguments});
      break;

    default:
      std::cout << "Error in sendRSG" << std::endl;
      break;
  }
  return generalUDPHandler(response);
}
int sendRLG(std::string input) {
  // TODO: fix this AND SEE CASE OF SYNTAX OF MESSAGE BEING WRONG
  size_t pos1 = input.find(' ');
  std::string plid = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  pos1 = input.find(' ');
  std::string letter = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  std::string trial = input;

  std::string arguments = "";
  int ret = playLetter(plid, letter, trial, arguments);
  std::string response;

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitString({"RLG", "OK", arguments});
      break;
    case SUCCESS_FINAL_GUESS:
      response = buildSplitString({"RLG", "WIN", arguments});
      break;
    case DUPLICATE_GUESS:
      response = buildSplitString({"RLG", "DUP", arguments});
      break;
    case WRONG_GUESS:
      response = buildSplitString({"RLG", "NOK", arguments});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitString({"RLG", "OVR", arguments});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitString({"RLG", "INV", arguments});
      break;
    case SYNTAX_ERROR:
      response = buildSplitString({"RLG", "ERR"});
      break;

    default:
      std::cout << "Error in sendRLG" << std::endl;
      break;
  }

  return generalUDPHandler(response);
}
int sendRWG(std::string input) {
  // TODO: fix this AND SEE CASE OF SYNTAX OF MESSAGE BEING WRONG
  size_t pos1 = input.find(' ');
  std::string plid = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  pos1 = input.find(' ');
  std::string word = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  std::string trial = input;

  std::string arguments = "";
  int ret = guessWord(plid, word, trial, arguments);
  std::string response;

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitString({"RLG", "WIN", arguments});
      break;
    case WRONG_GUESS:
      response = buildSplitString({"RLG", "NOK", arguments});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitString({"RLG", "OVR", arguments});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitString({"RLG", "INV", arguments});
      break;
    case SYNTAX_ERROR:
      response = buildSplitString({"RLG", "ERR"});
      break;

    default:
      std::cout << "Error in sendRWG" << std::endl;
      break;
  }

  return generalUDPHandler(response);

  return 0;
}
int sendRQT(std::string input) {
  std::string plid = input;

  int ret = closeGameSession(plid);
  std::string response;

  switch (ret) {
    case CLOSE_GAME_ERROR:
      response = buildSplitString({"RSG", "ERR"});
      break;
    case CLOSE_GAME_SUCCESS:
      response = buildSplitString({"RSG", "OK"});
      break;

    default:
      std::cout << "Error in sendRQT" << std::endl;
      break;
  }
  return generalUDPHandler(response);
}
int sendRRV(std::string input) { return 0; }

// Server message handlers
int handleSNG(struct protocolMessage message) {

  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string plid = body.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  return sendRSG(plid);
}
int handlePLG(struct protocolMessage message) {

  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string arguments = body.substr(pos1 + 1);
  arguments.erase(std::remove(arguments.begin(), arguments.end(), '\n'), arguments.end());

  sendRLG(arguments);
  return 0;
}
int handlePWG(struct protocolMessage message) {
  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string arguments = body.substr(pos1 + 1);
  arguments.erase(std::remove(arguments.begin(), arguments.end(), '\n'), arguments.end());

  sendRWG(arguments);
  return 0;
}
int handleQUT(struct protocolMessage message) {
  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string plid = body.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  return sendRQT(plid);
}
int handleREV(struct protocolMessage message) {
  sendRRV(message.code);
  std::cout << message.code << std::endl;
  return 0;
}

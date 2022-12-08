#include "server-protocol.h"

struct addrinfo *resUDP;
struct addrinfo hintsUDP;
int socketFdUDP;
char responseUDP[UDP_RECV_SIZE];
socklen_t addrlen;
char buffer[UDP_RECV_SIZE];
bool verbose;
char host[NI_MAXHOST], service[NI_MAXSERV]; // consts in <netdb.h>

// clang-format off
static responseHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

int setServerParameters(std::string filepath, bool vParam) {
  verbose = vParam;
  return setPath(filepath);
}

void createSocketUDP(std::string addr, std::string port) {
  socketFdUDP = newSocket(SOCK_DGRAM, addr, port, &hintsUDP, &resUDP);
  if (socketFdUDP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  while (true) {
    addrlen = sizeof(resUDP->ai_addr);
    if (recvfrom(socketFdUDP, buffer, UDP_RECV_SIZE, 0, resUDP->ai_addr, &addrlen) == -1) {
      exit(EXIT_FAILURE); // TODO: exit gracefully here
    }

    std::cout << "[INFO]: Received message: " << buffer;
    int errcode = getnameinfo(resUDP->ai_addr, addrlen, host, sizeof host, service, sizeof service, 0);
    if (verbose) {
      // TODO: put type of request
      if (errcode != 0) {
        std::cerr << VERBOSE_ERROR(errcode) << std::endl;
      } else {
        std::cout << VERBOSE_SUCCESS(host, service) << std::endl;
      }
    }

    generalUDPHandler(buffer);
    memset(buffer, 0, UDP_RECV_SIZE);
  }
}

int generalUDPHandler(std::string message) {
  struct protocolMessage response;
  const int ret = parseUDPMessage(message, response);
  if (ret == -1) {
    std::cerr << UDP_PARSE_ERROR << std::endl;
    return -1;
  }
  // TODO: try catch here, send ERR\n if the message is not valid
  return handleUDPClientMessage[response.first](response);
}

// Server message handlers
int handleSNG(struct protocolMessage message) { return sendRSG(message.second); }

int handlePLG(struct protocolMessage message) {
  std::string args = message.body.substr(message.firstPos + 1);
  args.erase(std::remove(args.begin(), args.end(), '\n'), args.end());
  return sendRLG(args);
}

int handlePWG(struct protocolMessage message) {
  std::string args = message.body.substr(message.firstPos + 1);
  args.erase(std::remove(args.begin(), args.end(), '\n'), args.end());
  return sendRWG(args);
}

int handleQUT(struct protocolMessage message) { return sendRQT(message.second); }

int handleREV(struct protocolMessage message) { return sendRRV(message.second); }

// UDP messages sent by the server
int sendRSG(std::string input) {
  const std::string plid = input;
  std::string gameInfo;
  std::string response;
  int ret = createGameSession(plid, gameInfo);
  switch (ret) {
    case CREATE_GAME_ERROR:
      response = buildSplitString({"RSG", "NOK"});
      break;
    case CREATE_GAME_SUCCESS:
      response = buildSplitString({"RSG", "OK", gameInfo});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitString({"RSG", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int sendRLG(std::string input) {
  const std::string errorMessage = buildSplitString({"RLG", "ERR"});
  const size_t pos1 = input.find(' ');
  if (pos1 == std::string::npos) {
    return sendUDPMessage(errorMessage, resUDP, socketFdUDP);
  }
  const std::string plid = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  const size_t pos2 = input.find(' ');
  if (pos2 == std::string::npos) {
    return sendUDPMessage(errorMessage, resUDP, socketFdUDP);
  }
  const std::string letter = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  const std::string trial = input;

  std::string guessInfo;
  std::string response;
  const int ret = playLetter(plid, letter, trial, guessInfo);

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitString({"RLG", "OK", guessInfo});
      break;
    case SUCCESS_FINAL_GUESS:
      response = buildSplitString({"RLG", "WIN", guessInfo});
      break;
    case DUPLICATE_GUESS:
      response = buildSplitString({"RLG", "DUP", guessInfo});
      break;
    case WRONG_GUESS:
      response = buildSplitString({"RLG", "NOK", guessInfo});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitString({"RLG", "OVR", guessInfo});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitString({"RLG", "INV", guessInfo});
      break;
    case SYNTAX_ERROR:
      response = buildSplitString({"RLG", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitString({"RLG", "ERR"});
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int sendRWG(std::string input) {
  const std::string errorMessage = buildSplitString({"RWG", "ERR"});
  const size_t pos1 = input.find(' ');
  if (pos1 == std::string::npos) {
    return sendUDPMessage(errorMessage, resUDP, socketFdUDP);
  }
  const std::string plid = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  const size_t pos2 = input.find(' ');
  if (pos2 == std::string::npos) {
    return sendUDPMessage(errorMessage, resUDP, socketFdUDP);
  }
  const std::string word = input.substr(0, pos1);
  input = input.substr(pos1 + 1);
  const std::string trial = input;

  std::string guessInfo;
  std::string response;
  int ret = guessWord(plid, word, trial, guessInfo);

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitString({"RWG", "WIN", guessInfo});
      break;
    case WRONG_GUESS:
      response = buildSplitString({"RWG", "NOK", guessInfo});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitString({"RWG", "OVR", guessInfo});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitString({"RWG", "INV", guessInfo});
      break;
    case SYNTAX_ERROR:
      response = buildSplitString({"RWG", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitString({"RWG", "ERR"});
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int sendRQT(std::string input) {
  const std::string plid = input;
  std::string response;
  int ret = closeGameSession(plid);

  switch (ret) {
    case CLOSE_GAME_SUCCESS:
      response = buildSplitString({"RQT", "OK"});
      break;
    case CLOSE_GAME_ERROR:
      response = buildSplitString({"RQT", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitString({"RQT", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int sendRRV(std::string input) {
  const std::string plid = input;
  std::string response;
  int ret = closeGameSession(plid); // FIXME: RRV SHOULD SEND THE PLAY'S WORD, NOT CLOSE THE TING

  switch (ret) {
    case CLOSE_GAME_SUCCESS:
      response = buildSplitString({"RRV", "OK"});
      break;
    case CLOSE_GAME_ERROR:
      response = buildSplitString({"RRV", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitString({"RRV", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

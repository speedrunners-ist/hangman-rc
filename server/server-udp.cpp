#include "server-protocol.h"

static struct addrinfo *resUDP;
static struct addrinfo hintsUDP;
static int socketFdUDP;
static char responseUDP[UDP_RECV_SIZE];
static socklen_t addrlen;
static char buffer[UDP_RECV_SIZE];
static bool verbose;
static char host[NI_MAXHOST], service[NI_MAXSERV]; // consts in <netdb.h>

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
  return setupWordList(filepath);
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

    generalUDPHandler(std::string(buffer));
    memset(buffer, 0, UDP_RECV_SIZE);
  }
}

int generalUDPHandler(std::string message) {
  struct protocolMessage response;
  int ret = parseUDPMessage(message, response);
  std::cout << "[INFO]: Parsed message, now on server UDP handler" << std::endl;
  if (ret == -1) {
    std::cerr << UDP_PARSE_ERROR << std::endl;
    return -1;
  }
  std::cout << "[INFO]: Message parsed successfully" << std::endl;
  std::cout << "[INFO]: Message type: " << response.first << std::endl;
  try {
    ret = handleUDPClientMessage[response.first](response);
  } catch (const std::out_of_range &oor) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }
  return ret;
}

// Server message handlers
int handleSNG(struct protocolMessage message) {
  std::cout << "[INFO]: Received SNG message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }
  const std::string plid = message.second;
  std::cout << "[INFO]: Player ID: " << plid << std::endl;

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
  std::cout << "[INFO]: Sending RSG message" << std::endl;
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handlePLG(struct protocolMessage message) {
  std::cout << "[INFO]: Received PLG message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }
  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());

  const std::string plid = message.second;
  std::string args = message.body.substr(message.secondPos + 1);
  const std::string letter = args.substr(0, 1);
  args = args.substr(2); // skip both the space and the letter
  const std::string trial = args;

  std::cout << "[INFO]: Received letter: " << letter << std::endl;
  std::cout << "[INFO]: Received trial: " << trial << std::endl;
  std::cout << "[INFO]: Received plid: " << plid << std::endl;

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

int handlePWG(struct protocolMessage message) {
  std::cout << "[INFO]: Received PWG message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }
  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());

  const std::string plid = message.second;
  std::string args = message.body.substr(message.secondPos + 1);
  if (args.find(' ') == std::string::npos) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }
  const std::string word = args.substr(0, args.find(' '));
  args = args.substr(args.find(' ') + 1);
  const std::string trial = args;

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

int handleQUT(struct protocolMessage message) {
  std::cout << "[INFO]: Received QUT message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.second;
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

int handleREV(struct protocolMessage message) {
  std::cout << "[INFO]: Received REV message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitString({"ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.second;
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

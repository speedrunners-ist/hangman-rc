#include "server-protocol.h"

// UDP related socket variables
struct addrinfo hintsUDP, *resUDP;
int socketFdUDP;
socklen_t addrlenUDP;
bool verboseUDP;
char hostUDP[NI_MAXHOST], serviceUDP[NI_MAXSERV]; // consts in <netdb.h>
char bufferUDP[UDP_RECV_SIZE];
char lastMessage[UDP_RECV_SIZE];
std::string response;
struct sigaction actUDP;

// clang-format off
responseHandler handleUDPClientMessage = {
  {"SNG", handleSNG},
  {"PLG", handlePLG},
  {"PWG", handlePWG},
  {"QUT", handleQUT},
  {"REV", handleREV}
};
// clang-format on

int setServerUDPParameters(std::string filepath, bool vParam) {
  verboseUDP = vParam;
  return setupWordList(filepath);
}

int createSocketUDP(struct peerInfo peer) {
  socketFdUDP = newSocket(SOCK_DGRAM, peer, &hintsUDP, &resUDP);
  if (socketFdUDP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  memset(&actUDP, 0, sizeof(actUDP));
  actUDP.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actUDP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    exit(EXIT_FAILURE); // TODO: exit gracefully here
  }

  return socketFdUDP;
}

int disconnectUDP() { return disconnectSocket(resUDP, socketFdUDP); }

int generalUDPHandler(struct peerInfo peer) {
  struct protocolMessage request;
  memset(lastMessage, 0, UDP_RECV_SIZE);
  if (createSocketUDP(peer) == -1) {
    return -1;
  }

  // Listen for incoming connections
  while (true) {
    memset(bufferUDP, 0, UDP_RECV_SIZE);

    addrlenUDP = sizeof(resUDP->ai_addr);
    if (recvfrom(socketFdUDP, bufferUDP, UDP_RECV_SIZE, 0, resUDP->ai_addr, &addrlenUDP) == -1) {
      exit(EXIT_FAILURE); // TODO: exit gracefully here
    }

    std::cout << "[INFO]: Received message: " << bufferUDP;
    if (verboseUDP) {
      int errcode =
          getnameinfo(resUDP->ai_addr, addrlenUDP, hostUDP, sizeof hostUDP, serviceUDP, sizeof serviceUDP, 0);
      if (errcode != 0) {
        std::cerr << VERBOSE_ERROR(errcode) << std::endl;
      } else {
        std::cout << VERBOSE_SUCCESS("UDP", hostUDP, serviceUDP) << std::endl;
      }
    }

    // Check if message is the same as the last one
    if (strcmp(bufferUDP, lastMessage) == 0) {
      sendUDPMessage(response, resUDP, socketFdUDP);
      continue;
    }

    memset(lastMessage, 0, UDP_RECV_SIZE);
    memcpy(lastMessage, bufferUDP, strlen(bufferUDP) + 1);

    if (parseUDPMessage(std::string(bufferUDP), request) == -1) {
      std::cerr << UDP_PARSE_ERROR << std::endl;
      sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
      continue;
    }

    try {
      handleUDPClientMessage[request.first](request);
    } catch (const std::bad_function_call &oor) {
      std::cerr << UDP_HANGMAN_ERROR << std::endl;
      sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
      continue;
    }
  }

  return 0;
}

// Server message handlers
int handleSNG(struct protocolMessage message) {
  std::cout << "[INFO]: Received SNG message" << std::endl;
  if (!message.body.substr(message.secondPos + 1).empty() || !hasPLIDformat(message.second)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"RSG", "ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.second;
  std::string gameInfo;
  const int ret = createGameSession(plid, gameInfo);
  switch (ret) {
    case CREATE_GAME_ERROR:
      response = buildSplitStringNewline({"RSG", "NOK"});
      break;
    case CREATE_GAME_SUCCESS:
      response = buildSplitStringNewline({"RSG", "OK", gameInfo});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RSG", "ERR"});
  }
  std::cout << "[INFO]: Sending RSG message" << std::endl;
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handlePLG(struct protocolMessage message) {
  std::cout << "[INFO]: Received PLG message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }
  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());

  const std::string plid = message.second;
  std::string args = message.body.substr(message.secondPos + 1);
  const std::string letter = args.substr(0, 1);
  args = args.substr(2); // skip both the space and the letter
  const std::string trial = args;

  std::string guessInfo;
  const int ret = playLetter(plid, letter, trial, guessInfo);

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitStringNewline({"RLG", "OK", guessInfo});
      break;
    case SUCCESS_FINAL_GUESS:
      response = buildSplitStringNewline({"RLG", "WIN", guessInfo});
      break;
    case DUPLICATE_GUESS:
      response = buildSplitStringNewline({"RLG", "DUP", guessInfo});
      break;
    case WRONG_GUESS:
      response = buildSplitStringNewline({"RLG", "NOK", guessInfo});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitStringNewline({"RLG", "OVR", guessInfo});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitStringNewline({"RLG", "INV", guessInfo});
      break;
    case SYNTAX_ERROR:
      response = buildSplitStringNewline({"RLG", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RLG", "ERR"});
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handlePWG(struct protocolMessage message) {
  std::cout << "[INFO]: Received PWG message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }
  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());

  const std::string plid = message.second;
  std::string args = message.body.substr(message.secondPos + 1);
  if (args.find(' ') == std::string::npos) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }
  const std::string word = args.substr(0, args.find(' '));
  args = args.substr(args.find(' ') + 1);
  const std::string trial = args;

  std::string guessInfo;
  const int ret = guessWord(plid, word, trial, guessInfo);

  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitStringNewline({"RWG", "WIN", guessInfo});
      break;
    case WRONG_GUESS:
      response = buildSplitStringNewline({"RWG", "NOK", guessInfo});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitStringNewline({"RWG", "OVR", guessInfo});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitStringNewline({"RWG", "INV", guessInfo});
      break;
    case SYNTAX_ERROR:
      response = buildSplitStringNewline({"RWG", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RWG", "ERR"});
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handleQUT(struct protocolMessage message) {
  std::cout << "[INFO]: Received QUT message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.second;
  const int ret = closeGameSession(plid);

  switch (ret) {
    case CLOSE_GAME_SUCCESS:
      response = buildSplitStringNewline({"RQT", "OK"});
      break;
    case CLOSE_GAME_ERROR:
      response = buildSplitStringNewline({"RQT", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RQT", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handleREV(struct protocolMessage message) {
  std::cout << "[INFO]: Received REV message" << std::endl;
  if (message.body.back() != '\n') {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.second;
  std::string word;
  const int ret = revealWord(plid, word);

  switch (ret) {
    case REVEAL_SUCCESS:
      response = buildSplitStringNewline({"RRV", word});
      break;
    case REVEAL_ERROR:
      response = buildSplitStringNewline({"RRV", "ERR"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RRV", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

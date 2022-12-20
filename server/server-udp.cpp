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

void signalHandlerUDP(int signum) {
  std::cout << std::endl << SIGNAL(signum) << std::endl;
  disconnectUDP();
  destroyTempFiles();
  std::cout << EXIT_PROGRAM << std::endl;
  exit(signum);
}

int setServerUDPParameters(std::string filepath, bool vParam) {
  verboseUDP = vParam;
  return setupWordList(filepath);
}

int createSocketUDP(peerInfo peer) {
  socketFdUDP = newSocket(SOCK_DGRAM, peer, &hintsUDP, &resUDP);
  if (socketFdUDP == -1) {
    std::cerr << SOCKET_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, signalHandlerUDP);
  signal(SIGTERM, signalHandlerUDP);

  memset(&actUDP, 0, sizeof(actUDP));
  actUDP.sa_handler = SIG_IGN;

  // Ignore SIGPIPE to avoid crashing when writing to a closed socket
  if (sigaction(SIGPIPE, &actUDP, NULL) == -1) {
    std::cerr << SIGACTION_ERROR << std::endl;
    disconnectUDP();
    return -1;
  }

  return socketFdUDP;
}

int disconnectUDP() { return disconnectSocket(resUDP, socketFdUDP); }

int generalUDPHandler(peerInfo peer) {
  protocolMessage request;
  memset(lastMessage, 0, UDP_RECV_SIZE);
  if (createSocketUDP(peer) == -1) {
    return -1;
  }

  // Listening for incoming connections
  while (true) {
    memset(bufferUDP, 0, UDP_RECV_SIZE);
    if (receiveUDPMessage(bufferUDP, UDP_RECV_SIZE, resUDP, socketFdUDP) == -1) {
      continue;
    }

    if (verboseUDP) {
      displayPeerInfo(resUDP, hostUDP, serviceUDP, "UDP");
    }

    // Check if message is the same as the last one
    if (strcmp(bufferUDP, lastMessage) == 0) {
      sendUDPMessage(response, resUDP, socketFdUDP);
      continue;
    }

    memset(lastMessage, 0, UDP_RECV_SIZE);
    memcpy(lastMessage, bufferUDP, strlen(bufferUDP) + 1);

    if (parseMessage(std::string(bufferUDP), request) == -1) {
      std::cerr << PARSE_ERROR << std::endl;
      sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
      continue;
    }

    if (verboseUDP) {
      std::cout << "[INFO]: Received the following message: " << request.body << std::endl;
    }
    messageUDPHandler(request, handleUDPClientMessage, socketFdUDP, resUDP);
  }

  return 0;
}

// Server message handlers
int handleSNG(protocolMessage message) {
  if (!validArgsAmount(message.body, SNG_ARGS) || !validPlayerID(message.status)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"RSG, ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.status;
  std::string gameInfo;
  const int ret = createGameSession(plid, gameInfo);
  switch (ret) {
    case CREATE_GAME_ERROR:
      response = buildSplitStringNewline({"RSG", "ERR"});
      break;
    case CREATE_GAME_ONGOING:
      response = buildSplitStringNewline({"RSG", "NOK"});
      break;
    case CREATE_GAME_SUCCESS:
      response = buildSplitStringNewline({"RSG", "OK", gameInfo});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handlePLG(protocolMessage message) {
  const std::string plid = message.status;
  if (!validArgsAmount(message.body, PLG_ARGS) || !validPlayerID(plid)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());
  const std::string args = message.args;
  const std::string letter = args.substr(0, 1);
  const std::string trial = args.substr(2);

  if (isdigit(letter[0]) || args[1] != ' ' || !isNumber(trial)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

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
      response = buildSplitStringNewline({"ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handlePWG(protocolMessage message) {
  const std::string plid = message.status;
  if (!validArgsAmount(message.body, PWG_ARGS) || !validPlayerID(plid)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  message.body.erase(std::remove(message.body.begin(), message.body.end(), '\n'), message.body.end());
  const std::string args = message.args;
  const std::string word = args.substr(0, args.find(' '));
  const std::string trial = args.substr(args.find(' ') + 1);
  if (!hasWordFormat(word) || !isNumber(trial)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  std::string guessInfo;
  const int ret = guessWord(plid, word, trial, guessInfo);
  switch (ret) {
    case SUCCESS_GUESS:
      response = buildSplitStringNewline({"RWG", "WIN", guessInfo});
      break;
    case DUPLICATE_GUESS:
      response = buildSplitStringNewline({"RWG", "DUP", guessInfo});
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
      response = buildSplitStringNewline({"ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handleQUT(protocolMessage message) {
  if (!validArgsAmount(message.body, QUT_ARGS) || !validPlayerID(message.status)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"RQT", "ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.status;
  const int ret = closeGameSession(plid);
  switch (ret) {
    case CLOSE_GAME_SUCCESS:
      response = buildSplitStringNewline({"RQT", "OK"});
      break;
    case CLOSE_GAME_ERROR:
      response = buildSplitStringNewline({"RQT", "NOK"});
      break;
    default:
      std::cerr << INTERNAL_ERROR << std::endl;
      response = buildSplitStringNewline({"RQT", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int handleREV(protocolMessage message) {
  if (!validArgsAmount(message.body, REV_ARGS) || !validPlayerID(message.status)) {
    std::cerr << UDP_RESPONSE_ERROR << std::endl;
    return sendUDPMessage(buildSplitStringNewline({"ERR"}), resUDP, socketFdUDP);
  }

  const std::string plid = message.status;
  std::string word;
  const int ret = revealWord(plid, word);
  switch (ret) {
    case REVEAL_SUCCESS:
      response = buildSplitStringNewline({"RRV", word});
      break;
    case REVEAL_ERROR:
    default:
      response = buildSplitStringNewline({"RRV", "ERR"});
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

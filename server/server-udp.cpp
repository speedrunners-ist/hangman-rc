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
static commandHandler handleUDPClientMessage = {
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
  int ret = parseUDPMessage(message, response);
  if (ret == -1) {
    std::cerr << UDP_PARSE_ERROR << std::endl;
    return -1;
  }
  return handleUDPClientMessage[response.code](response);
}

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
  return sendRLG(arguments);
}

int handlePWG(struct protocolMessage message) {
  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string arguments = body.substr(pos1 + 1);
  arguments.erase(std::remove(arguments.begin(), arguments.end(), '\n'), arguments.end());
  return sendRWG(arguments);
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
  std::string body = message.body;
  // TODO: check if body is empty
  const size_t pos1 = body.find(' ');

  std::string plid = body.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());

  return sendRRV(plid);
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
      return -1;
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
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
      return -1;
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);
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
      response = buildSplitString({"RWG", "WIN", arguments});
      break;
    case WRONG_GUESS:
      response = buildSplitString({"RWG", "NOK", arguments});
      break;
    case WRONG_FINAL_GUESS:
      response = buildSplitString({"RWG", "OVR", arguments});
      break;
    case TRIAL_MISMATCH:
      response = buildSplitString({"RWG", "INV", arguments});
      break;
    case SYNTAX_ERROR:
      response = buildSplitString({"RWG", "ERR"});
      break;

    default:
      std::cout << "Error in sendRWG" << std::endl;
      return -1;
  }

  return sendUDPMessage(response, resUDP, socketFdUDP);

  return 0;
}

int sendRQT(std::string input) {
  std::string plid = input;

  int ret = closeGameSession(plid);
  std::string response;

  switch (ret) {
    case CLOSE_GAME_ERROR:
      response = buildSplitString({"RQT", "ERR"});
      break;
    case CLOSE_GAME_SUCCESS:
      response = buildSplitString({"RQT", "OK"});
      break;

    default:
      std::cout << "Error in sendRQT" << std::endl;
      return -1;
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

int sendRRV(std::string input) {
  std::string plid = input;

  int ret = closeGameSession(plid);
  std::string response;

  switch (ret) {
    case CLOSE_GAME_ERROR:
      response = buildSplitString({"RRV", "ERR"});
      break;
    case CLOSE_GAME_SUCCESS:
      response = buildSplitString({"RRV", "OK"});
      break;

    default:
      std::cout << "Error in sendRRV" << std::endl;
      return -1;
  }
  return sendUDPMessage(response, resUDP, socketFdUDP);
}

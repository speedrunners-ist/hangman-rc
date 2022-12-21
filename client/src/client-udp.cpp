#include "client-protocol.h"

// clang-format off
responseHandler handleUDPServerMessage = {
  {"RSG", handleRSG},
  {"RLG", handleRLG},
  {"RWG", handleRWG},
  {"RQT", handleRQT},
  {"RRV", handleRRV}
};

std::map<std::string, int> expectedResponseArgs = {
  {"RSG OK", 4},  {"RSG NOK", 2}, {"RSG ERR", 2},
  {"RLG WIN", 3}, {"RLG DUP", 3}, {"RLG NOK", 3}, {"RLG OVR", 3}, {"RLG INV", 3}, {"RLG ERR", 2},
  {"RWG WIN", 3}, {"RWG NOK", 3}, {"RWG OVR", 3}, {"RWG INV", 3}, {"RWG ERR", 2},
  {"RQT OK", 2},  {"RQT NOK", 2}, {"RQT ERR", 2}
};
// clang-format on

// Since the protocol may answer with ERR if the game is already finished and we try to guess the word again,
// we need to send back a STA request to check if the game is over. If the answer is an RST FIN, we need to
// reset the game.
bool checkFinishedGame() {
  if (sendTCPMessage(buildSplitStringNewline({"STA", getPlayerID()}), getServerInfoTCP(), getSocketFdTCP()) ==
      -1) {
    return false;
  }
  std::string responseMessage;
  if (receiveTCPMessage(responseMessage, TCP_DEFAULT_ARGS, getSocketFdTCP()) == -1) {
    return false;
  }
  const std::string expectedMessage = "RST FIN ";
  return responseMessage == expectedMessage;
}

int generalUDPHandler(std::string message, size_t maxBytes) {
  char responseMessage[maxBytes + 1];
  memset(responseMessage, 0, maxBytes + 1);
  protocolMessage response;
  if (sendUDPMessage(message, getServerInfoUDP(), getSocketFdUDP()) == -1) {
    return -1;
  }
  if (receiveUDPMessage(responseMessage, maxBytes, getServerInfoUDP(), getSocketFdUDP()) == -1) {
    return -1;
  }
  if (parseMessage(responseMessage, response) == -1) {
    std::cerr << UDP_HANGMAN_ERROR << std::endl;
    return -1;
  }

  if (response.command != "RLG OK") { // edge case where the number of arguments is uncertain
    try {
      if (!validArgsAmount(response.body, expectedResponseArgs[response.command])) {
        std::cerr << UDP_HANGMAN_ERROR << std::endl;
        return -1;
      }
    } catch (const std::exception &e) {
      std::cerr << UDP_HANGMAN_ERROR << std::endl;
      return -1;
    }
  }
  return messageUDPHandler(response, handleUDPServerMessage);
}

/**
 * Server response handlers
 */

int handleRSG(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    std::vector<int> args;
    if (!gatherResponseArguments(response.args, args, RSG_ARGS)) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    const int wordLength = args[0];
    const int availableMistakes = args[1];
    const int expectedMistakes = initialAvailableMistakes(wordLength);
    if (wordLength <= 0 || expectedMistakes != availableMistakes) {
      std::cerr << RSG_ERROR << std::endl;
      return -1;
    }
    createGame(args, getPlayerID());
    const std::string word = getWord();
    std::cout << RSG_OK(availableMistakes, word) << std::endl;
    return 0;
  }
  if (response.status == "NOK") {
    std::cout << RSG_NOK << std::endl;
    return 0;
  }
  if (response.status == "ERR") {
    std::cout << RSG_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRLG(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    std::vector<int> args;
    if (!gatherResponseArguments(response.args, args, RLG_ARGS)) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    const int trial = args[0];
    const int n = args[1];
    if (trial != getTrials() + 1 || n <= 0 || n > getWordLength()) {
      std::cerr << RLG_ERROR << std::endl;
      return -1;
    }
    std::string oArgs = buildSplitString({std::to_string(trial), std::to_string(n)});
    response.args = response.args.substr(response.args.find(oArgs) + oArgs.length() + 1);
    return playCorrectGuess(response.args, n);
  }
  if (response.status == "WIN") {
    playCorrectFinalGuess();
    resetGame();
    std::cout << RLG_WIN(getWord()) << std::endl;
    return 0;
  }
  if (response.status == "DUP") {
    std::cout << RLG_DUP << std::endl;
    return 0;
  }
  if (response.status == "NOK") {
    playIncorrectGuess();
    std::cout << RLG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  }
  if (response.status == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RLG_OVR << std::endl;
    return 0;
  }
  if (response.status == "INV") {
    std::cout << RLG_INV << std::endl;
    return 0;
  }
  if (response.status == "ERR") {
    std::cout << RLG_ERR << std::endl;
    if (isActive() && checkFinishedGame()) {
      resetGame();
    }
    return 0;
  }
  return -1;
}

int handleRWG(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "WIN") {
    std::vector<int> args;
    if (!gatherResponseArguments(response.args, args, RWG_ARGS)) {
      std::cerr << RWG_ERROR << std::endl;
      return -1;
    }
    const int trial = args[0];
    if (trial != getTrials() + 1) {
      std::cerr << RWG_ERROR << std::endl;
      return -1;
    }
    playCorrectFinalWordGuess();
    resetGame();
    std::cout << RWG_WIN(getWord()) << std::endl;
    return 0;
  }
  if (response.status == "NOK") {
    playIncorrectGuess();
    std::cout << RWG_NOK(getAvailableMistakes()) << std::endl;
    return 0;
  }
  if (response.status == "OVR") {
    playIncorrectGuess();
    resetGame();
    std::cout << RWG_OVR << std::endl;
    return 0;
  }
  if (response.status == "INV") {
    std::cout << RWG_INV << std::endl;
    return 0;
  }
  if (response.status == "ERR") {
    std::cout << RWG_ERR << std::endl;
    if (isActive() && checkFinishedGame()) {
      resetGame();
    }
    return 0;
  }
  return -1;
}

int handleRQT(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  if (response.status == "OK") {
    resetGame();
    std::cout << RQT_OK << std::endl;
    return 0;
  }
  if (response.status == "ERR") {
    std::cout << RQT_ERR << std::endl;
    return 0;
  }
  return -1;
}

int handleRRV(protocolMessage response) {
  if (response.request != getExpectedMessage()) {
    std::cerr << UNEXPECTED_MESSAGE << std::endl;
    return -1;
  }

  std::cout << RRV_OK(response.status) << std::endl;
  return 0;
}

/**
 * Client request handlers
 */

int sendSNG(messageInfo info) {
  if (!validArgsAmount(info.input, START_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl; // TODO
    return -1;
  }

  const size_t pos1 = info.input.find(' ');
  std::string plid = info.input.substr(pos1 + 1);
  plid.erase(std::remove(plid.begin(), plid.end(), '\n'), plid.end());
  if (!validPlayerID(plid)) {
    return -1;
  }
  setPlayerID(plid);
  const std::string message = buildSplitStringNewline({"SNG", plid});
  setExpectedMessage("RSG");
  return generalUDPHandler(message, RSG_BYTES);
}

int sendPLG(messageInfo info) {
  if (!validArgsAmount(info.input, PLAY_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  if (getPlayerID().empty()) {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }

  const size_t pos1 = info.input.find(' ');
  std::string letter = info.input.substr(pos1 + 1);
  letter.erase(std::remove(letter.begin(), letter.end(), '\n'), letter.end());
  if (letter.length() != 1 || !std::isalpha(letter[0])) {
    std::cerr << EXPECTED_LETTER_ERROR << std::endl;
    return -1;
  }
  const std::string message =
      buildSplitStringNewline({"PLG", getPlayerID(), letter, std::to_string(getTrials() + 1)});
  setLastGuess(letter[0]);
  setExpectedMessage("RLG");
  return generalUDPHandler(message, RLG_BYTES);
}

int sendPWG(messageInfo info) {
  if (!validArgsAmount(info.input, GUESS_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  if (getPlayerID().empty()) {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }

  const size_t pos1 = info.input.find(' ');
  std::string guess = info.input.substr(pos1 + 1);
  guess.erase(std::remove(guess.begin(), guess.end(), '\n'), guess.end());
  if (guess.length() != getWordLength()) {
    std::cerr << EXPECTED_WORD_DIF_LEN_ERROR(getWordLength()) << std::endl;
    return -1;
  }
  const std::string message =
      buildSplitStringNewline({"PWG", getPlayerID(), guess, std::to_string(getTrials() + 1)});
  setLastWordGuess(guess);
  setExpectedMessage("RWG");
  return generalUDPHandler(message, RWG_BYTES);
}

int sendQUT(messageInfo info) {
  if (!validArgsAmount(info.input, QUIT_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  if (getPlayerID().empty()) {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }

  const std::string command = info.input.substr(0, info.input.find('\n'));
  const std::string message = buildSplitStringNewline({"QUT", getPlayerID()});
  setExpectedMessage("RQT");
  if (command == "quit") {
    return generalUDPHandler(message, RQT_BYTES);
  }
  return generalUDPHandler(message, RQT_BYTES) == 0 ? EXIT_HANGMAN : -1;
}

int sendREV(messageInfo info) {
  if (!validArgsAmount(info.input, REVEAL_ARGS)) {
    std::cout << UNEXPECTED_COMMAND << std::endl;
    return -1;
  }
  if (getPlayerID().empty()) {
    std::cerr << NO_PLAYER_ERROR << std::endl;
    return -1;
  }

  const std::string message = buildSplitStringNewline({"REV", getPlayerID()});
  setExpectedMessage("RRV");
  return generalUDPHandler(message, RRV_BYTES);
}

#include "server-protocol.h"

int main(int argc, char *argv[]) {
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;
  std::string filePath;
  bool verbose = false;
  int opt;

  while ((opt = getopt(argc, argv, "p:v")) != -1) {
    switch (opt) {
      case 'p':
        GSport = optarg;
        break;
      case 'v':
        verbose = true;
        break;
      default:
        std::cout << WRONG_ARGS_ERROR << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  filePath = argv[optind];
  std::cout << STARTING_SERVER << std::endl;
  if (setServerUDPParameters(filePath, verbose) == -1) {
    std::cerr << STARTING_SERVER_ERROR << std::endl;
    exit(EXIT_FAILURE);
  }

  setServerTCPParameters(verbose);

  const struct peerInfo peer = {"", GSport};
  pid_t pid = fork();

  if (pid == 0) {
    generalTCPHandler(peer);
  } else if (pid > 0) {
    generalUDPHandler(peer);
  }

  std::cout << EXIT_PROGRAM << std::endl;
  exit(EXIT_SUCCESS);
}

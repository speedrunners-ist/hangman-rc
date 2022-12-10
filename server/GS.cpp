#include "server-protocol.h"

int main(int argc, char *argv[]) {

  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;
  std::string filePath;
  bool verbose = false;
  int pid;

  int opt;
  std::string GSPort = DEFAULT_GSPORT;

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

  pid = fork();

  if (pid == 0) {
    createSocketTCP("", GSport);
  } else if (pid > 0) {
    createSocketUDP("", GSport);
  }
}

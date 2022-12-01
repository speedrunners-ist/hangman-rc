#include "server-protocol.h"

int main(int argc, char *argv[]) {

  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;
  std::string filePath;

  bool portSet = false;
  bool verbose = false;

  // TODO: maybe consider using getopt instead?
  int i;
  for (i = 1; i < argc; i++) {

    if (strcmp(argv[i], "-v") == 0) {
      if (verbose)
        std::cout << "[WARN]: -v flag already set. Ignoring..." << std::endl;
      else
        verbose = true;
      continue;
    }

    if (strcmp(argv[i], "-p") == 0) {
      if (portSet) {
        std::cout << "[ERR]: Port defined multiple times. Exiting..." << std::endl;
        return -1;
      }
      if (i + 1 >= argc) {
        std::cout << "[ERR]: Invalid input. Expected port number." << std::endl;
        return -1;
      }
      if (validatePort(argv[i + 1]) == -1) {
        std::cout << "[ERR]: Invalid input. Expected valid port number." << std::endl;
        return -1;
      }
      GSport = argv[i + 1];
      portSet = true;
      i++;
      continue;
    }

    if (i == 1) {
      if (std::ifstream(argv[i])) {
        filePath = argv[i];
        continue;
      } else {
        std::cout << "[ERR]: Invalid input. Expected valid file path." << std::endl;
        return -1;
      }
    }

    std::cout << "[ERR]: Invalid input. Exiting..." << std::endl;
    return -1;
  }

  if (filePath.empty()) {
    std::cout << "[ERR]: Invalid input. Expected file path." << std::endl;
    return -1;
  }

  std::cout << "Starting server..." << std::endl;

  setServerParamaters(filePath, verbose);

  createSocketUDP("", GSport);
}

#include "server-protocol.h"

int main(int argc, char *argv[]) {

  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;
  std::string filePath;

  bool verbose = false;

  // TODO: maybe consider using getopt instead?
  int i;
  for (i = 0; i < argc; i++) {

    if (strcmp(argv[i], "-v") == 0) {
      verbose = true;
      continue;
    }

    if (strcmp(argv[i], "-p") == 0) {
      if (i + 1 >= argc) {
        std::cout << "[ERR]: Invalid input. Expected port number." << std::endl;
        return -1;
      }
      if (validatePort(argv[i + 1]) == -1) {
        std::cout << "[ERR]: Invalid input. Expected valid port number." << std::endl;
        return -1;
      }
      GSport = argv[i + 1];
      continue;
    }

    if (i == 1)
      filePath = argv[i];
  }

  if (filePath.empty()) {
    std::cout << "[ERR]: Invalid input. Expected file path." << std::endl;
    return -1;
  }

  std::cout << "Starting server..." << std::endl;

  setServerParamaters(filePath, verbose);

  createSocketUDP("", GSport);
}

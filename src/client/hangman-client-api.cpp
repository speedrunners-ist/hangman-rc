#include <../hangman-common.h>

// TODO: standardize error messages with macros
// TODO: in order for the program to exit gracefully, we always need to close any open sockets!!

int newSocket(struct addrinfo *serverInfo, int type, std::string addr, std::string port) {
  const int fd = socket(AF_INET, type, 0);
  if (fd == -1) {
    // FIXME: should we really exit here?
    std::cout << "[ERR]: Failed to create socket. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = type;

  const int status = getaddrinfo(addr.c_str(), port.c_str(), &hints, &serverInfo);
  if (status != 0) {
    std::cout << "[ERR]: Failed to get address info. Exiting." << std::endl;
    return -1;
  }
  return fd;
}

int main(int argc, char *argv[]) {
  // TODO: maybe consider a parsing function?
  // command format: ./player [-n GSIP] [-p GSport]
  // GSIP: IP address of the game server
  // GSport: port number of the game server
  // both arguments are optional, with default values being DEFAULT_GSIP and DEFAULT_GSPORT

  int opt;
  std::string GSIP = DEFAULT_GSIP;
  std::string GSport = DEFAULT_GSPORT;

  while ((opt = getopt(argc, argv, "n:p:")) != -1) {
    switch (opt) {
      case 'n':
        // TODO: probably macro this
        GSIP = std::string(optarg);
        if (!std::string(optarg).ends_with("tecnico.ulisboa.pt")) {
          GSIP = std::string(optarg) + ".tecnico.ulisboa.pt";
        }
        // TODO: it's also possible to receive an actual ipv4 address, so we've got to check for
        // that (and if it's well formatted and such)
        break;
      case 'p':
        // TODO: check if the port is valid?
        GSport = std::string(optarg);
        break;
      default:
        std::cerr << "[ERR] Usage: ./player [-n GSIP] [-p GSport]" << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  struct addrinfo *serverInfo;
  const int fd = newSocket(serverInfo, SOCK_DGRAM, GSIP, GSport);
  if (fd == -1) {
    std::cout << "[ERR]: Failed to create socket. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }

  int res = mkdir("hints", 0700);
  if (res == -1 && errno != EEXIST) {
    // if the directory can't be created and it doesn't already exist
    std::cout << "[ERR]: Failed to create hints directory. Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }

  // TODO: implement the client's main loop

  // delete the directory and its contents
  system("rm -rf hints");

  close(fd);
  freeaddrinfo(serverInfo);
  exit(EXIT_SUCCESS);
}
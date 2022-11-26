#ifndef HANGMAN_COMMON_H
#define HANGMAN_COMMON_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_GSIP "127.0.0.1"
#define DEFAULT_GSPORT "58045"

#define MAX_USER_INPUT 1024

#define UDP_TRIES 3
#define UDP_RECV_SIZE 4096

#endif

// Below, function prototypes

/*
 * @brief: Amount of wrong guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
unsigned int initialAvailableMistakes(unsigned int wordLength);

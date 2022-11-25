#ifndef HANGMAN_COMMON_H
#define HANGMAN_COMMON_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#define DEFAULT_GSIP "127.0.0.1"
#define DEFAULT_GSPORT "58050"
// TODO: change this to the correct port number (i.e 58000 + group number)

#endif

// Below, function prototypes

/*
 * @brief: Amount of wrong guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
unsigned int initialAvailableMistakes(unsigned int wordLength);

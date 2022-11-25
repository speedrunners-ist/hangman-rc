#ifndef HANGMAN_COMMON_H
#define HANGMAN_COMMON_H

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <string>
#include <map>
#include <vector>

#define DEFAULT_GSIP "127.0.0.1"
#define DEFAULT_GSPORT "58050" // TODO: change this to the correct port number (i.e 58000 + group number)

#endif

// Below, function prototypes

/*
 * @brief: Amount of guesses a player can make before failing.
 *
 * @param: wordLength: length of the word to guess
 * @return: number of guesses
 */
int initialGuesses(int wordLength);

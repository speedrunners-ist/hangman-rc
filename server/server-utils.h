#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "common/common.h"

#include <ctime>
#include <dirent.h>
#include <filesystem>

typedef struct {
  int score[10];
  char PLID[10][20];
  char name[10][20];
  int n_succ[10];
  int n_tot[10];
  int n_scores;
} SCORELIST;

int findOccurringGame(char *PLID, char *fname);
int FindLastGame(char *PLID, char *fname);
int FindTopScores(SCORELIST *list);

int createGameFile(std::string plid, std::string word, std::string hint);
int appendGameFile(std::string plid, std::string code, std::string play);
int transferGameFile(std::string plid, std::string status);

int parseTime(tm *time, std::string &filename);

#endif
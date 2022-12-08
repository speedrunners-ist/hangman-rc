#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "common/common.h"

#include <dirent.h>

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

#endif
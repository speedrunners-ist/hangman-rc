#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "common/common.h"

#include <ctime>
#include <dirent.h>
#include <filesystem>

#define PLID_GAMES_PATH(plid) "games/" + plid
#define ONGOING_GAMES_PATH(plid) "games/GAME_" + plid + ".txt"
#define FINISHED_GAMES_PATH(plid, time) PLID_GAMES_PATH(plid) + "/GAME_" + time + ".txt"
#define SCORES_PATH "scores/scoreboard.txt"
#define SCORES_HEADER "SCORE PLAYER     WORD                      GOOD TRIALS  TOTAL TRIALS"
#define MAX_CACHE_SIZE 128

typedef struct {
  int score[10];
  char PLID[10][20];
  char name[10][20];
  int n_succ[10];
  int n_tot[10];
  int n_scores;
} SCORELIST;

int createGameFile(std::string plid, std::string word, std::string hint);
int appendGameFile(std::string plid, std::string code, std::string play);
int transferGameFile(std::string plid);
int appendScoreFile(int score, std::string scoreline);
void writeScoreFileHeader(std::fstream &file, std::vector<std::string> lines);

#endif

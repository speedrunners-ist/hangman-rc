#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "common/common.h"

#include <ctime>
#include <dirent.h>
#include <filesystem>

#define GAMES_PATH "server/games/"
#define PLID_GAMES_PATH(plid) "server/games/" + plid
#define ONGOING_GAMES_PATH(plid) "server/games/GAME_" + plid + ".txt"
#define FINISHED_GAMES_PATH(plid, time) PLID_GAMES_PATH(plid) + "/GAME_" + time + ".txt"
#define SCORES_PATH "server/scores/scoreboard.txt"
#define TOP_10_HEADER "TOP 10 SCORES"
#define SCORES_HEADER "SCORE PLAYER     WORD                      GOOD TRIALS  TOTAL TRIALS"
#define MAX_CACHE_SIZE 128
#define TIME_FORMAT "%Y-%m-%d_%H-%M-%S"
#define FILE_PATH(filename) "server/hints/" + filename

int createGameFile(std::string plid, std::string word, std::string hint);
int appendGameFile(std::string plid, std::string code, std::string play);
int transferGameFile(std::string plid);
int appendScoreFile(int score, std::string scoreline);
void writeScoreFileHeader(std::fstream &file, std::vector<std::string> lines);

void getLastFinishedGame(std::string plid, std::string &filepath);

#endif

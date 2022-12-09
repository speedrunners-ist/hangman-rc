#include "server-utils.h"

int findOccurringGame(char *PLID, char *fname) {

  struct dirent **filelist;
  int n_entries, found;
  char dirname[20];
  std::string auxStr;

  n_entries = scandir("GAMES/", &filelist, 0, alphasort);

  if (n_entries <= 0)
    return 0;

  found = 0;

  // TODO: improve this
  while (n_entries--) {
    if (filelist[n_entries]->d_name[0] != '.') {
      auxStr = filelist[n_entries]->d_name;
      auxStr = auxStr.substr(5, 6);
      if (strcmp(auxStr.c_str(), PLID) == 0) {
        sprintf(fname, "GAMES/%s", filelist[n_entries]->d_name);
        found = 1;
      }
      free(filelist[n_entries]);
      if (found)
        break;
    }
  }
  free(filelist);
  return found;
}

int FindLastGame(char *PLID, char *fname) {
  struct dirent **filelist;
  int n_entries, found;
  char dirname[20];

  sprintf(dirname, "GAMES/%s/", PLID);

  n_entries = scandir(dirname, &filelist, 0, alphasort);

  found = 0;

  if (n_entries <= 0)
    return 0;

  while (n_entries--) {
    if (filelist[n_entries]->d_name[0] != '.') {
      sprintf(fname, "GAMES/%s/%s", PLID, filelist[n_entries]->d_name);
      found = 1;
    }
    free(filelist[n_entries]);
    if (found)
      break;
  }
  free(filelist);
  return found;
}

int FindTopScores(SCORELIST *list) {
  struct dirent **filelist;
  int n_entries, i_file;
  char fname[50];
  FILE *fp;

  n_entries = scandir("SCORES/", &filelist, 0, alphasort);

  i_file = 0;

  if (n_entries < 0)
    return 0;

  while (n_entries--) {
    if (filelist[n_entries]->d_name[0] != '.') {
      sprintf(fname, "SCORES/%s", filelist[n_entries]->d_name);
      fp = fopen(fname, "r");
      if (fp != NULL) {
        fscanf(fp, "%d %s %s %d %d", &list->score[i_file], list->PLID[i_file], list->name[i_file],
               &list->n_succ[i_file], &list->n_tot[i_file]);
        fclose(fp);
        ++i_file;
      }
    }
    free(filelist[n_entries]);
    if (i_file == 10)
      break;
  }
  free(filelist);
  list->n_scores = i_file;
  return i_file;
}

int createGameFile(std::string plid, std::string word, std::string hint) {

  std::fstream file;

  std::string dir = "GAMES/GAME_" + plid + ".txt";
  std::string content = buildSplitString({word, hint});
  // TODO: create these folders in the client directory
  file.open(dir, std::ios::out | std::ios::in | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }

  file.write(content.c_str(), content.size());

  file.close();

  return 0;
}

int appendGameFile(std::string plid, std::string code, std::string play) {
  std::fstream file;
  std::string dir = "GAMES/GAME_" + plid + ".txt";
  std::string content = buildSplitString({code, play});

  // append to the file
  file.open(dir, std::ios::app);

  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }

  file.write(content.c_str(), content.size());

  file.close();

  return 0;
}

int transferGameFile(std::string plid, std::string status) {
  std::fstream oldfile;
  std::fstream newfile;
  std::string filename;

  time_t now = time(0);

  tm *time = localtime(&now);

  parseTimeGame(time, filename);

  filename = filename.append("_" + status);

  std::string olddir = "GAMES/GAME_" + plid + ".txt";
  std::string newdir = "GAMES/" + plid;

  // create the directory

  mkdir(newdir.c_str(), 0777);

  newdir += "/" + filename + ".txt";

  try {
    std::filesystem::copy(olddir, newdir);
    std::filesystem::remove(olddir);
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}

int parseTimeGame(tm *time, std::string &filename) {

  std::string year = std::to_string(1900 + time->tm_year);
  std::string month = std::to_string(1 + time->tm_mon);
  std::string day = std::to_string(time->tm_mday);
  std::string hour = std::to_string(time->tm_hour);
  std::string min = std::to_string(time->tm_min);
  std::string sec = std::to_string(time->tm_sec);
  std::vector<std::string> timeVec = {year, month, day, hour, min, sec};

  for (auto &t : timeVec) {
    filename = filename.append(t);
    if (t == day)
      filename = filename.append("_");
  }
  return 0;
}

int createScoreFile(std::string plid, std::string score, std::string content) {
  std::fstream file;

  std::string date = "";

  time_t now = time(0);

  tm *time = localtime(&now);

  parseTimeScore(time, date);

  std::string dir = "SCORES/" + score + "_" + plid + "_" + date + ".txt";

  file.open(dir, std::ios::in | std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }

  file.write(content.c_str(), content.size());

  file.close();

  return 0;
}

int parseTimeScore(tm *time, std::string &filename) {

  std::string year = std::to_string(1900 + time->tm_year);
  std::string month = std::to_string(1 + time->tm_mon);
  std::string day = std::to_string(time->tm_mday);
  std::string hour = std::to_string(time->tm_hour);
  std::string min = std::to_string(time->tm_min);
  std::string sec = std::to_string(time->tm_sec);
  std::vector<std::string> timeVec = {day, month, year, hour, min, sec};

  for (auto &t : timeVec) {
    filename = filename.append(t);
    if (t == year)
      filename = filename.append("_");
  }
  return 0;
}
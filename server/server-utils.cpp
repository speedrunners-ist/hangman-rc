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
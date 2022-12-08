#include "server-utils.h"

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
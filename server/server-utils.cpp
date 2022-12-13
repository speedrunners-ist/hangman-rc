#include "server-utils.h"

int createGameFile(std::string plid, std::string word, std::string hint) {
  // create games directory if it doesn't exist
  std::filesystem::path dir(GAMES_PATH);
  if (!std::filesystem::exists(dir)) {
    std::filesystem::create_directory(dir);
  }
  std::ofstream file(ONGOING_GAMES_PATH(plid), std::ios::trunc);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }
  const std::string content = buildSplitStringNewline({word, hint});
  file.write(content.c_str(), (ssize_t)content.size());
  file.close();
  return 0;
}

int appendGameFile(std::string plid, std::string code, std::string play) {
  std::ofstream file(ONGOING_GAMES_PATH(plid), std::ios::app);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }
  std::string content = buildSplitStringNewline({code, play});
  file.write(content.c_str(), (ssize_t)content.size());
  file.close();
  return 0;
}

int transferGameFile(std::string plid) {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80]; // TODO: macro this
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), TIME_FORMAT, timeinfo);
  std::string time(buffer);

  std::string filename;
  std::string oldFile = ONGOING_GAMES_PATH(plid);
  std::string newFile = FINISHED_GAMES_PATH(plid, time);

  // create the player's directory if it doesn't exist
  std::filesystem::path dir(PLID_GAMES_PATH(plid));
  if (!std::filesystem::exists(dir)) {
    std::filesystem::create_directory(dir);
  }

  try {
    std::filesystem::copy(oldFile, newFile);
    std::filesystem::remove(oldFile);
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << e.what() << std::endl; // something went wrong, macro this
    return -1;
  }

  return 0;
}

int appendScoreFile(int score, std::string scoreline) {
  std::fstream file(SCORES_PATH, std::ios::in | std::ios::out);
  if (!file.is_open()) {
    std::cerr << FILE_OPEN_ERROR << std::endl;
    return -1;
  }

  // If the file is empty, we need to write the header and add the scoreline
  if (file.peek() == std::ifstream::traits_type::eof()) {
    writeScoreFileHeader(file, {scoreline});
    file.close();
    return 0;
  }

  /*
   * Otherwise, we need to find if the current score fits in the top 10
   * For that, we check the score field for each line in the file - if it's
   * lower than the current score, we insert the scoreline in that position,
   * and shift the rest of the lines down
   * At the end, if there are more than 10 scores, we remove the last one
   */
  std::string line;
  std::vector<std::string> lines;
  int i = 0;
  while (std::getline(file, line)) {
    if (i >= 3) {
      // We do this to avoid the header and the separators
      lines.push_back(line);
    }
    ++i;
  }

  // Otherwise, we need to find the position where the score fits
  int pos = 0;
  for (auto &l : lines) {
    std::string lineScore = l.substr(0, l.find(' '));
    if (score > std::stoi(lineScore)) {
      break;
    }
    ++pos;
  }

  // Insert the scoreline in the correct position
  lines.insert(lines.begin() + pos, scoreline);
  if (lines.size() > 10) {
    // If there are more than 10 scores, we must remove the last one
    lines.pop_back();
  }

  writeScoreFileHeader(file, lines);
  file.close();
  return 0;
}

void writeScoreFileHeader(std::fstream &file, std::vector<std::string> lines) {
  file.clear();
  file.seekp(0, std::ios::beg);
  file << TOP_10_HEADER << std::endl;
  file << std::string(std::string(SCORES_HEADER).size(), '-') << std::endl;
  file << SCORES_HEADER << std::endl;
  for (auto &l : lines) {
    file << l << std::endl;
  }
}

int getLastFinishedGame(std::string plid, std::string &filePath) {

  std::filesystem::path dir(PLID_GAMES_PATH(plid));
  if (!std::filesystem::exists(dir)) {
    return -1;
  }

  std::vector<std::string> files;
  for (const auto &entry : std::filesystem::directory_iterator(dir)) {
    files.push_back(entry.path());
  }

  if (files.size() == 0) {
    return -1;
  }

  std::sort(files.begin(), files.end());
  // return name
  filePath = files.back().erase(0, files.back().find_last_of('/') + 1);
  return 0;
}

int createPlaceholderState(std::string plid, std::string filePath) {
  std::filesystem::path dir(TMP_DIR);

  try {
    if (!std::filesystem::exists(dir))
      std::filesystem::create_directory(dir);
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  // create file TMP_PATH(plid) if it doesn't exist
  std::fstream newFile(TMP_PATH(plid), std::ios::out);
  if (!newFile.is_open()) {
    return -1;
  }

  std::fstream oldFile(filePath, std::ios::in);
  if (!oldFile.is_open()) {
    return -1;
  }

  /*
   * We'll need to copy the file line by line, trimming the first, since the first
   * line has the word and hint (which should be hidden from the player until completion)
   */
  std::string line;
  std::getline(oldFile, line); // skip the first line
  while (std::getline(oldFile, line)) {
    newFile << line << std::endl;
  }

  newFile.close();
  oldFile.close();
  return 0;
}

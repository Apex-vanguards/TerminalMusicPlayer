#ifndef UI_HPP
#define UI_HPP

#include "player.hpp"
#include "theme.hpp"
#include <deque>
#include <string>
#include <vector>
#define MAXPREVSONGCNT 100

class UI {
private:
  Player &player;
  ThemeManager themes;
  std::vector<std::string> tracks;
  std::vector<std::string> trackNames;
  std::deque<int> prevTracks;
  int selected = 0;
  int current = -1; /// WARNING: this can lead to memory leaks
  std::string musicDir;
  PlayerModeState state = NORMAL;
  bool isShuffle = false;

  void ScanLibrary(void);
  void Draw(void);
  void PlaySelected(void);
  void PlayNext(void);
  void PlayPrev(void);
  const char *getMode(void);
  void NextMode(void);
  bool isValidMusicIndex(int index);

public:
  UI(Player &p, const std::string &dir);
  void Start();
};

#endif // UI_HPP

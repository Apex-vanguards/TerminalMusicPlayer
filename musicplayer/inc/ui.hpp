#ifndef UI_HPP
#define UI_HPP

#include "player.hpp"
#include <string>
#include <vector>

class UI {
private:
  Player &player;
  std::vector<std::string> tracks;
  std::vector<std::string> trackNames;
  int selected = 0;
  int current = -1;
  std::string musicDir;
  bool isShuffled = false;

  void ScanLibrary();
  void Draw();
  void PlaySelected();
  void PlayNext();
  void PlayPrev();

public:
  UI(Player &p, const std::string &dir);
  void Start();
};

#endif // UI_HPP

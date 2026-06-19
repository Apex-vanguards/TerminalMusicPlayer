#ifndef UI_HPP
#define UI_HPP

#include "player.hpp"
#include <string>
#include <vector>

class UI {
private:
  Player &player; // reference, not a copy
  std::vector<std::string> tracks;
  std::vector<std::string> trackNames;
  int selected = 0;  // cursor position in list
  int current = -1;  // currently loaded/playing track index
  std::string musicDir;

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

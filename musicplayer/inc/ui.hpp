#ifndef UI_HPP
#define UI_HPP

#include "player.hpp"
#include "playlist.hpp"
#include "theme.hpp"
#include <deque>
#include <string>
#include <vector>
#define MAXPREVSONGCNT 100

class UI {
private:
  PlaylistManager plMgr;
  int plSelected = 0;
  bool inPlaylistMode = false;
  bool inPlaylistSelector = false;
  int plSelectorIdx = 0;
  std::vector<std::string> plSelectorList;
  std::string flashMsg;
  int flashTicks = 0;
  Player &player;
  ThemeManager themes;
  std::vector<std::string> THEMES = {"cappuccino", "dark", "dracula", "gruvbox",
                                     "nord"};
  int theme_idx = 0;
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
  void DrawPlaylistSelector(void);
  void PlaySelected(void);
  void PlayNext(void);
  void PlayPrev(void);
  const char *getMode(void);
  void NextMode(void);
  bool isValidMusicIndex(int index);
  void ChangeThemeNext(void);
  void getPlaylists(void);
  void getMusicsFromPlaylist(std::string &playlist);
  void toggleAllLoadingDirs(void);
  void AddCurrentToPlaylist();
  void RemoveFromPlaylist();
  void SavePlaylist();
  void NewPlaylist();
  void PlayFromPlaylist();
  void OpenPlaylistSelector();
  void SetFlash(const std::string &msg, int ticks = 8);

public:
  UI(Player &p, const std::string &dir);
  void Start();
};

#endif // UI_HPP

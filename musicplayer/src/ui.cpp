#include "../inc/ui.hpp"
#include "../inc/theme.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <ncurses.h>
#include <random>
#include <stdexcept>
#include <sys/stat.h>

bool shouldGetAllDirs = true;

static bool HasAudioExt(const std::string &name) {
  static const char *exts[] = {".mp3", ".wav", ".flac", ".ogg",
                               ".m4a", ".aac", ".opus"};
  for (auto ext : exts) {
    size_t elen = strlen(ext);
    if (name.size() >= elen &&
        strcasecmp(name.c_str() + name.size() - elen, ext) == 0) {
      return true;
    }
  }
  return false;
}

UI::UI(Player &p, const std::string &dir) : player(p), musicDir(dir) {
  ScanLibrary();
}

static void ScanDirRec(const std::string &baseDir, const std::string &relDir,
                       std::vector<std::string> &names,
                       std::vector<std::string> &paths) {
  std::string currentDir = baseDir + (relDir.empty() ? "" : "/" + relDir);

  DIR *d = opendir(currentDir.c_str());
  if (!d)
    return;

  struct dirent *entry;
  while ((entry = readdir(d)) != nullptr) {
    std::string name = entry->d_name;
    if (name == "." || name == "..")
      continue;

    std::string relPath = relDir.empty() ? name : relDir + "/" + name;
    std::string fullPath = baseDir + "/" + relPath;

    struct stat st;
    if (stat(fullPath.c_str(), &st) != 0)
      continue;

    if (S_ISDIR(st.st_mode) && shouldGetAllDirs) {
      ScanDirRec(baseDir, relPath, names, paths);
    } else if (HasAudioExt(name)) {
      names.push_back(relPath);
      paths.push_back(fullPath);
    }
  }

  closedir(d);
}

void UI::ScanLibrary() {
  tracks.clear();
  trackNames.clear();

  std::vector<std::string> names;
  std::vector<std::string> paths;

  ScanDirRec(musicDir, "", names, paths);

  if (isShuffle) {
    isShuffle = false;
    std::random_device rdv;
    std::mt19937 g(rdv());
    std::vector<size_t> idx(names.size());
    for (size_t i = 0; i < idx.size(); ++i)
      idx[i] = i;
    std::shuffle(idx.begin(), idx.end(), g);

    std::vector<std::string> shuffledNames, shuffledPaths;
    for (size_t i : idx) {
      shuffledNames.push_back(names[i]);
      shuffledPaths.push_back(paths[i]);
    }
    names = std::move(shuffledNames);
    paths = std::move(shuffledPaths);
  }

  trackNames = std::move(names);
  tracks = std::move(paths);
}

bool UI::isValidMusicIndex(int index) {
  return (bool)0 <= index && index < (int)tracks.size();
}

void UI::PlaySelected() {
  if (tracks.empty())
    return;
  if (isValidMusicIndex(current)) {
    prevTracks.push_back(current);
  }
  current = selected;
  player.SetFilePath(tracks[current]);
  player.LoadCurrent();
  player.Play();
  while (prevTracks.size() > MAXPREVSONGCNT) {
    assert(prevTracks.size() > 0);
    prevTracks.pop_front();
  }
}

void UI::NextMode() {
  if (state == NORMAL) {
    state = RANDOM;
  } else if (state == RANDOM) {
    state = REPEAT;
  } else if (state == REPEAT) {
    state = NORMAL;
  }
}

void UI::toggleAllLoadingDirs(void) {
  shouldGetAllDirs = !shouldGetAllDirs;
  ScanLibrary();
}

void UI::PlayNext() {
  if (state == NORMAL) {
    if (tracks.empty()) {
      return;
    }
    selected = (current + 1) % (int)tracks.size();
    PlaySelected();
  }
  if (state == RANDOM) {
    if (tracks.empty()) {
      return;
    }
    selected = rand() % (int)tracks.size();
    PlaySelected();
  }
  if (state == REPEAT) {
    if (tracks.empty()) {
      return;
    }
    selected = current;
    PlaySelected();
  }
}

void UI::PlayPrev() {
  if (tracks.empty() || prevTracks.empty())
    return;
  assert(!prevTracks.empty());
  assert(prevTracks.size() > 0);
  selected = prevTracks.back();
  prevTracks.pop_back();
  PlaySelected();
  assert(!prevTracks.empty());
  prevTracks.pop_back();
}

const char *UI::getMode() {
  if (state == RANDOM) {
    return "RANDOM";
  } else if (state == NORMAL) {
    return "NORMAL";
  } else {
    return "REPEAT";
  }
}

void UI::Draw() {
  erase();

  attron(themes.pair(CP::Title) | themes.attr(CP::Title));
  printw("Terminal Music Player  -  %s  [%s]\n", musicDir.c_str(),
         inPlaylistMode ? ("PLAYLIST: " + plMgr.Active().name).c_str()
                        : "LIBRARY");
  attroff(themes.pair(CP::Title) | themes.attr(CP::Title));

  printw("up/down: select  enter: play  p: pause  s: stop  n: next  b: prev  "
         "r: shuffle  a: toggle All dirs  q: quit l: next theme  m: next mode "
         "TAB: playlist mod  e: add to last visited playlist d: delete w: save "
         "c: new "
         "playlist\n");
  printw("Mode: %s\n", getMode());
  printw("---------------------------------------------------------------\n");

  if (inPlaylistMode) {
    if (plMgr.Active().IsEmpty()) {
      printw(
          "Playlist is empty. Add music to the playlist from the library.\n");
    } else {
      int maxY, maxX;
      getmaxyx(stdscr, maxY, maxX);
      (void)maxX;
      int listRows = maxY - 7;
      if (listRows < 1)
        listRows = 1;
      int sz = plMgr.Active().Size();
      int start = 0;
      if (plSelected >= listRows)
        start = plSelected - listRows + 1;
      int end = std::min(sz, start + listRows);
      for (int i = start; i < end; ++i) {
        bool isSel = (i == plSelected);
        const auto &e = plMgr.Active().entries[i];
        if (isSel) {
          attron(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
          printw("  %s\n", e.name.c_str());
          attroff(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
        } else {
          attron(themes.pair(CP::Default));
          printw("  %s\n", e.name.c_str());
          attroff(themes.pair(CP::Default));
        }
      }
    }
  } else {

    if (tracks.empty()) {
      printw("No audio files found in %s\n", musicDir.c_str());
    } else {
      int maxY, maxX;
      getmaxyx(stdscr, maxY, maxX);
      (void)maxX;
      int listRows = maxY - 7;
      if (listRows < 1)
        listRows = 1;

      int start = 0;
      if (selected >= listRows)
        start = selected - listRows + 1;
      int end = std::min((int)tracks.size(), start + listRows);

      for (int i = start; i < end; ++i) {
        bool isSelected = (i == selected);
        bool isPlaying = (i == current);
        const char *marker = isPlaying ? "> " : "  ";

        if (isSelected) {
          attron(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
          printw("%s%s\n", marker, trackNames[i].c_str());
          attroff(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
        } else if (isPlaying) {
          attron(themes.pair(CP::Status) | themes.attr(CP::Status));
          printw("%s%s\n", marker, trackNames[i].c_str());
          attroff(themes.pair(CP::Status) | themes.attr(CP::Status));
        } else {
          attron(themes.pair(CP::Default));
          printw("%s%s\n", marker, trackNames[i].c_str());
          attroff(themes.pair(CP::Default));
        }
      }
    }
  }

  printw("---------------------------------------------------------------\n");

  if (current >= 0 && current < (int)tracks.size()) {
    double pos = player.GetCursorSeconds();
    double len = player.GetLengthSeconds();
    int pm = (int)pos / 60, ps = (int)pos % 60;
    int lm = (int)len / 60, ls = (int)len % 60;

    attron(themes.pair(CP::Status) | themes.attr(CP::Status));
    printw("Now playing: %s   [%02d:%02d / %02d:%02d]  %s\n",
           trackNames[current].c_str(), pm, ps, lm, ls,
           player.IsPlaying() ? "(playing)" : "(paused/stopped)");
    attroff(themes.pair(CP::Status) | themes.attr(CP::Status));
  } else {
    printw("Nothing playing\n");
  }

  if (flashTicks > 0) {
    attron(themes.pair(CP::Status));
    printw("%s\n", flashMsg.c_str());
    attroff(themes.pair(CP::Status));
    flashTicks--;
  }

  refresh();
}

void UI::ChangeThemeNext() {
  if (theme_idx + 1 == (int)THEMES.size()) {
    theme_idx = 0;
  } else {
    theme_idx++;
  }
  themes.setTheme(THEMES[theme_idx]);
  themes.apply();
}

void UI::Start() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(150);

  start_color();
  themes.loadFromDisk();
  themes.setTheme(THEMES[theme_idx]);
  themes.apply();

  bool running = true;
  while (running) {
    Draw();

    int ch = getch();
    switch (ch) {
    case 'q':
    case 'Q':
      running = false;
      break;
    case KEY_UP:
      if (inPlaylistMode) {
        if (plMgr.Active().Size() > 0)
          plSelected =
              (plSelected - 1 + plMgr.Active().Size()) % plMgr.Active().Size();
      } else {
        if (!tracks.empty())
          selected = (selected - 1 + (int)tracks.size()) % (int)tracks.size();
      }
      break;
    case KEY_DOWN:
      if (inPlaylistMode) {
        if (plMgr.Active().Size() > 0)
          plSelected = (plSelected + 1) % plMgr.Active().Size();
      } else {
        if (!tracks.empty())
          selected = (selected + 1) % (int)tracks.size();
      }
      break;
    case '\n':
    case KEY_ENTER:
      if (inPlaylistMode)
        PlayFromPlaylist();
      else
        PlaySelected();
      break;
    case 'p':
    case 'P':
      player.TogglePause();
      break;
    case 's':
    case 'S':
      player.Stop();
      break;
    case 'n':
    case 'N':
      PlayNext();
      break;
    case 'b':
    case 'B':
      PlayPrev();
      break;
    case 'r':
    case 'R':
      isShuffle = true;
      ScanLibrary();
      selected = 0;
      break;
    case 'a':
    case 'A':
      toggleAllLoadingDirs();
      break;
    case 'm':
    case 'M':
      NextMode();
      break;
    case 'l':
    case 'L':
      ChangeThemeNext();
      break;
    case '\t':
      OpenPlaylistSelector();
      break;
    case 'e':
    case 'E':
      AddCurrentToPlaylist();
      break;
    case 'd':
    case 'D':
      RemoveFromPlaylist();
      break;
    case 'w':
    case 'W':
      SavePlaylist();
      break;
    case 'c':
    case 'C':
      NewPlaylist();
      break;
    case 'k':
    case 'K':
      inPlaylistMode = false;
    default:
      break;
    }

    if (current >= 0 && !tracks.empty()) {
      if (!player.IsPlaying() && player.GetCursorSeconds() > 0.0 &&
          player.GetLengthSeconds() > 0.0 &&
          player.GetCursorSeconds() >= player.GetLengthSeconds() - 0.2) {
        PlayNext();
      }
    }
  }

  endwin();
}

void UI::SetFlash(const std::string &msg, int ticks) {
  flashMsg = msg;
  flashTicks = ticks;
}

void UI::AddCurrentToPlaylist() {
  if (tracks.empty() || !isValidMusicIndex(selected))
    return;
  plMgr.AddToActive(trackNames[selected], tracks[selected]);
  SavePlaylist();
  SetFlash("Added: " + trackNames[selected]);
}

void UI::RemoveFromPlaylist() {
  if (!inPlaylistMode || plMgr.Active().IsEmpty())
    return;
  plMgr.RemoveFromActive(plSelected);
  SavePlaylist();
  if (plSelected >= plMgr.Active().Size() && plSelected > 0)
    plSelected--;
  SetFlash("Deleted from Playlist.");
}

void UI::SavePlaylist() {
  bool ok = plMgr.SaveActive();
  SetFlash(ok ? "Saved: " + plMgr.Active().name : "Saving failed");
}

void UI::NewPlaylist() {
  echo();
  curs_set(1);

  timeout(-1);
  char nameBuffer[256];
  nameBuffer[0] = '\0';

  clear();
  mvprintw(0, 0, "Enter playlist name: ");
  refresh();
  getnstr(nameBuffer, (int)sizeof(nameBuffer) - 1);

  timeout(150);
  noecho();
  curs_set(0);

  std::string newName(nameBuffer);
  if (!newName.empty()) {
    plMgr.NewPlaylist(newName);
    plSelected = 0;
    SavePlaylist();
    SetFlash("Playlist created: " + newName);
  }

  clear();
}

void UI::PlayFromPlaylist() {
  if (plMgr.Active().IsEmpty())
    return;
  if (plSelected < 0 || plSelected >= plMgr.Active().Size())
    return;
  const auto &e = plMgr.Active().entries[plSelected];
  player.SetFilePath(e.path);
  player.LoadCurrent();
  player.Play();
}

void UI::DrawPlaylistSelector() {
  erase();

  attron(themes.pair(CP::Title) | themes.attr(CP::Title));
  printw("Playlist Sec\n");
  attroff(themes.pair(CP::Title) | themes.attr(CP::Title));

  printw("up/down: move enter: selecet  ESC: cancel\n");
  printw("---------------------------------------------------------------\n");

  if (plSelectorList.empty()) {
    printw("You havent playlist\n");
  } else {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    (void)maxX;
    int listRows = maxY - 6;
    if (listRows < 1)
      listRows = 1;
    int sz = (int)plSelectorList.size();
    int start = 0;
    if (plSelectorIdx >= listRows)
      start = plSelectorIdx - listRows + 1;
    int end = std::min(sz, start + listRows);

    for (int i = start; i < end; ++i) {
      std::string displayName =
          std::filesystem::path(plSelectorList[i]).stem().string();
      bool isSel = (i == plSelectorIdx);
      if (isSel) {
        attron(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
        printw("  > %s\n", displayName.c_str());
        attroff(themes.pair(CP::Highlight) | themes.attr(CP::Highlight));
      } else {
        attron(themes.pair(CP::Default));
        printw("    %s\n", displayName.c_str());
        attroff(themes.pair(CP::Default));
      }
    }
  }

  printw("---------------------------------------------------------------\n");
  refresh();
}

void UI::OpenPlaylistSelector() {
  plSelectorList = plMgr.ListSavedPlaylists();
  plSelectorIdx = 0;
  inPlaylistSelector = true;

  while (inPlaylistSelector) {
    DrawPlaylistSelector();
    int ch = getch();
    int sz = (int)plSelectorList.size();

    switch (ch) {
    case KEY_UP:
      if (sz > 0)
        plSelectorIdx = (plSelectorIdx - 1 + sz) % sz;
      break;
    case KEY_DOWN:
      if (sz > 0)
        plSelectorIdx = (plSelectorIdx + 1) % sz;
      break;
    case '\n':
    case KEY_ENTER:
      if (sz > 0 && plSelectorIdx < sz) {
        plMgr.LoadPlaylist(plSelectorList[plSelectorIdx]);
        plSelected = 0;
        inPlaylistMode = true;
        SetFlash("Playlist dowloaded: " + plMgr.Active().name);
      }
      inPlaylistSelector = false;
      break;
    case 27:
      inPlaylistSelector = false;
      break;
    default:
      break;
    }
  }
}

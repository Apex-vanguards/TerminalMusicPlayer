#include "../inc/ui.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <dirent.h>
#include <ncurses.h>
#include <random>
#include <stdexcept>
#include <sys/stat.h>

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

void UI::ScanLibrary() {
  tracks.clear();
  trackNames.clear();

  DIR *d = opendir(musicDir.c_str());
  if (!d)
    return;

  std::vector<std::string> names;
  struct dirent *entry;
  while ((entry = readdir(d)) != nullptr) {
    std::string name = entry->d_name;
    if (name == "." || name == "..")
      continue;

    std::string fullPath = musicDir + "/" + name;
    struct stat st;
    if (stat(fullPath.c_str(), &st) != 0)
      continue;
    if (S_ISDIR(st.st_mode))
      continue;
    if (!HasAudioExt(name))
      continue;

    names.push_back(name);
  }
  closedir(d);

  // std::sort(names.begin(), names.end());

  if (isShuffled) {
    std::random_device rdv;
    std::mt19937 g(rdv());
    std::shuffle(names.begin(), names.end(), g);
  }

  for (auto &name : names) {
    trackNames.push_back(name);
    tracks.push_back(musicDir + "/" + name);
  }
}

void UI::PlaySelected() {
  if (tracks.empty())
    return;
  current = selected;
  player.SetFilePath(tracks[current]);
  player.LoadCurrent();
  player.Play();
}

void UI::PlayNext() {
  if (tracks.empty())
    return;
  selected = (current + 1) % (int)tracks.size();
  PlaySelected();
}

void UI::PlayPrev() {
  if (tracks.empty())
    return;
  selected = (current - 1 + (int)tracks.size()) % (int)tracks.size();
  PlaySelected();
}

void UI::Draw() {
  erase();
  printw("Terminal Music Player  -  %s\n", musicDir.c_str());
  printw("up/down: select  enter: play  p: pause  s: stop  n: next  b: prev  "
         "r: shuffle  a: alpha  q: quit\n");
  printw("Mode: %s\n", isShuffled ? "SHUFFLE" : "SAME");
  printw("---------------------------------------------------------------\n");

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
    if (selected >= listRows) {
      start = selected - listRows + 1;
    }
    int end = std::min((int)tracks.size(), start + listRows);

    for (int i = start; i < end; ++i) {
      bool isSelected = (i == selected);
      bool isPlaying = (i == current);

      if (isSelected)
        attron(A_REVERSE);

      const char *marker = isPlaying ? "> " : "  ";
      printw("%s%s\n", marker, trackNames[i].c_str());

      if (isSelected)
        attroff(A_REVERSE);
    }
  }

  printw("---------------------------------------------------------------\n");
  if (current >= 0 && current < (int)tracks.size()) {
    double pos = player.GetCursorSeconds();
    double len = player.GetLengthSeconds();
    int pm = (int)pos / 60, ps = (int)pos % 60;
    int lm = (int)len / 60, ls = (int)len % 60;
    printw("Now playing: %s   [%02d:%02d / %02d:%02d]  %s\n",
           trackNames[current].c_str(), pm, ps, lm, ls,
           player.IsPlaying() ? "(playing)" : "(paused/stopped)");
  } else {
    printw("Nothing playing\n");
  }

  refresh();
}

void UI::Start() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(150);

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
      if (!tracks.empty())
        selected = (selected - 1 + (int)tracks.size()) % (int)tracks.size();
      break;
    case KEY_DOWN:
      if (!tracks.empty())
        selected = (selected + 1) % (int)tracks.size();
      break;
    case '\n':
    case KEY_ENTER:
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
      isShuffled = true;
      ScanLibrary();
      selected = 0;
      break;
    case 'a':
    case 'A':
      isShuffled = false;
      ScanLibrary();
      selected = 0;
      break;
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

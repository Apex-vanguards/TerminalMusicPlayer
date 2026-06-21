#include "../inc/theme.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

std::string ThemeManager::lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::string ThemeManager::trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  size_t b = s.find_last_not_of(" \t\r\n");
  return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

short ThemeManager::resolveColor(const std::string &s) {
  std::string c = lower(trim(s));
  if (c == "black")
    return COLOR_BLACK;
  if (c == "red")
    return COLOR_RED;
  if (c == "green")
    return COLOR_GREEN;
  if (c == "yellow")
    return COLOR_YELLOW;
  if (c == "blue")
    return COLOR_BLUE;
  if (c == "magenta" || c == "purple" || c == "pink")
    return COLOR_MAGENTA;
  if (c == "cyan")
    return COLOR_CYAN;
  return COLOR_WHITE;
}

attr_t ThemeManager::resolveAttr(const std::string &s) {
  attr_t r = A_NORMAL;
  std::istringstream ss(s);
  std::string tok;
  while (std::getline(ss, tok, '|')) {
    std::string t = lower(trim(tok));
    if (t == "bold")
      r |= A_BOLD;
    else if (t == "dim")
      r |= A_DIM;
    else if (t == "reverse")
      r |= A_REVERSE;
    else if (t == "underline")
      r |= A_UNDERLINE;
    else if (t == "blink")
      r |= A_BLINK;
  }
  return r;
}

ThemeManager::ThemeManager() : active_("dark") { registerBuiltins(); }

std::string ThemeManager::themesDir() {
  const char *h = std::getenv("HOME");
  return std::string(h ? h : "/root") + "/.config/CatMusicPlayer/themes";
}

void ThemeManager::loadFromDisk() {
  ensureDirs();
  copyDefaultThemes();

  for (const auto &entry : fs::directory_iterator(themesDir())) {
    if (entry.path().extension() == ".theme") {
      Theme t = parse(entry.path().string());
      if (!t.name.empty())
        themes_[lower(t.name)] = t;
    }
  }
}

bool ThemeManager::setTheme(const std::string &name) {
  std::string key = lower(name);
  if (themes_.count(key) == 0)
    return false;
  active_ = key;
  return true;
}

void ThemeManager::apply() const {
  auto it = themes_.find(active_);
  if (it == themes_.end())
    return;
  for (const auto &[id, tc] : it->second.colors)
    init_pair((short)id, tc.fg, tc.bg);
}

int ThemeManager::pair(CP cp) const { return COLOR_PAIR((int)cp); }

attr_t ThemeManager::attr(CP cp) const {
  auto it = themes_.find(active_);
  if (it == themes_.end())
    return A_NORMAL;
  auto ai = it->second.attrs.find((int)cp);
  return ai != it->second.attrs.end() ? ai->second : A_NORMAL;
}

std::vector<std::string> ThemeManager::list() const {
  std::vector<std::string> out;
  for (const auto &[k, t] : themes_)
    out.push_back(t.name);
  return out;
}

const std::string &ThemeManager::current() const { return active_; }

Theme ThemeManager::parse(const std::string &path) const {
  static const std::unordered_map<std::string, int> sec2id = {
      {"default", (int)CP::Default},     {"title", (int)CP::Title},
      {"border", (int)CP::Border},       {"highlight", (int)CP::Highlight},
      {"progress", (int)CP::Progress},   {"progressbg", (int)CP::ProgressBg},
      {"status", (int)CP::Status},       {"artist", (int)CP::Artist},
      {"timestamp", (int)CP::TimeStamp}, {"inactive", (int)CP::Inactive},
  };

  Theme theme;
  std::ifstream f(path);
  if (!f.is_open())
    return theme;

  int curSec = -1;
  short curFg = COLOR_WHITE, curBg = COLOR_BLACK;
  attr_t curAttr = A_NORMAL;

  auto flush = [&]() {
    if (curSec < 0)
      return;
    theme.colors[curSec] = {curFg, curBg};
    theme.attrs[curSec] = curAttr;
  };

  std::string line;
  while (std::getline(f, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#')
      continue;

    if (line.front() == '[' && line.back() == ']') {
      flush();
      std::string sec = lower(trim(line.substr(1, line.size() - 2)));
      auto it = sec2id.find(sec);
      curSec = (it != sec2id.end()) ? it->second : -1;
      curFg = COLOR_WHITE;
      curBg = COLOR_BLACK;
      curAttr = A_NORMAL;
      continue;
    }

    size_t eq = line.find('=');
    if (eq == std::string::npos)
      continue;
    std::string key = lower(trim(line.substr(0, eq)));
    std::string val = trim(line.substr(eq + 1));

    if (key == "name")
      theme.name = val;
    else if (key == "fg")
      curFg = resolveColor(val);
    else if (key == "bg")
      curBg = resolveColor(val);
    else if (key == "attr")
      curAttr = resolveAttr(val);
  }
  flush();

  if (theme.name.empty())
    theme.name = fs::path(path).stem().string();

  return theme;
}

void ThemeManager::ensureDirs() { fs::create_directories(themesDir()); }

void ThemeManager::copyDefaultThemes() {
  const std::string src = "/usr/share/CatMusicPlayer/themes";
  const std::string dst = themesDir();

  if (!fs::exists(src))
    return;

  for (const auto &entry : fs::directory_iterator(src)) {
    if (entry.path().extension() != ".theme")
      continue;
    fs::path target = fs::path(dst) / entry.path().filename();
    if (!fs::exists(target)) {
      fs::copy_file(entry.path(), target);
    }
  }
}

void ThemeManager::registerBuiltins() {
  auto make = [&](const std::string &name,
                  std::unordered_map<int, ThemeColor> colors,
                  std::unordered_map<int, attr_t> attrs) {
    Theme t;
    t.name = name;
    t.colors = std::move(colors);
    t.attrs = std::move(attrs);
    themes_[lower(name)] = t;
  };

  make("Dark",
       {
           {(int)CP::Default, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Title, {COLOR_CYAN, COLOR_BLACK}},
           {(int)CP::Border, {COLOR_BLUE, COLOR_BLACK}},
           {(int)CP::Highlight, {COLOR_BLACK, COLOR_CYAN}},
           {(int)CP::Progress, {COLOR_BLACK, COLOR_BLUE}},
           {(int)CP::ProgressBg, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Status, {COLOR_GREEN, COLOR_BLACK}},
           {(int)CP::Artist, {COLOR_CYAN, COLOR_BLACK}},
           {(int)CP::TimeStamp, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Inactive, {COLOR_WHITE, COLOR_BLACK}},
       },
       {
           {(int)CP::Title, A_BOLD},
           {(int)CP::Highlight, A_BOLD},
           {(int)CP::TimeStamp, A_DIM},
           {(int)CP::Inactive, A_DIM},
       });
  make("Cappuccino",
       {
           {(int)CP::Default, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Title, {COLOR_YELLOW, COLOR_BLACK}},
           {(int)CP::Border, {COLOR_MAGENTA, COLOR_BLACK}},
           {(int)CP::Highlight, {COLOR_BLACK, COLOR_YELLOW}},
           {(int)CP::Progress, {COLOR_BLACK, COLOR_GREEN}},
           {(int)CP::ProgressBg, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Status, {COLOR_CYAN, COLOR_BLACK}},
           {(int)CP::Artist, {COLOR_MAGENTA, COLOR_BLACK}},
           {(int)CP::TimeStamp, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Inactive, {COLOR_WHITE, COLOR_BLACK}},
       },
       {
           {(int)CP::Title, A_BOLD},
           {(int)CP::Highlight, A_BOLD},
           {(int)CP::TimeStamp, A_DIM},
           {(int)CP::Inactive, A_DIM},
       });

  make("Nord",
       {
           {(int)CP::Default, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Title, {COLOR_BLUE, COLOR_BLACK}},
           {(int)CP::Border, {COLOR_CYAN, COLOR_BLACK}},
           {(int)CP::Highlight, {COLOR_BLACK, COLOR_BLUE}},
           {(int)CP::Progress, {COLOR_BLACK, COLOR_CYAN}},
           {(int)CP::ProgressBg, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Status, {COLOR_GREEN, COLOR_BLACK}},
           {(int)CP::Artist, {COLOR_CYAN, COLOR_BLACK}},
           {(int)CP::TimeStamp, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Inactive, {COLOR_WHITE, COLOR_BLACK}},
       },
       {
           {(int)CP::Title, A_BOLD},
           {(int)CP::Highlight, A_BOLD},
           {(int)CP::TimeStamp, A_DIM},
           {(int)CP::Inactive, A_DIM},
       });

  make("Gruvbox",
       {
           {(int)CP::Default, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Title, {COLOR_YELLOW, COLOR_BLACK}},
           {(int)CP::Border, {COLOR_RED, COLOR_BLACK}},
           {(int)CP::Highlight, {COLOR_BLACK, COLOR_RED}},
           {(int)CP::Progress, {COLOR_BLACK, COLOR_YELLOW}},
           {(int)CP::ProgressBg, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Status, {COLOR_GREEN, COLOR_BLACK}},
           {(int)CP::Artist, {COLOR_RED, COLOR_BLACK}},
           {(int)CP::TimeStamp, {COLOR_WHITE, COLOR_BLACK}},
           {(int)CP::Inactive, {COLOR_WHITE, COLOR_BLACK}},
       },
       {
           {(int)CP::Title, A_BOLD},
           {(int)CP::Highlight, A_BOLD},
           {(int)CP::TimeStamp, A_DIM},
           {(int)CP::Inactive, A_DIM},
       });
}

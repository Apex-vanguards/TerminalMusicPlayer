#pragma once

#include <ncurses.h>
#include <string>
#include <unordered_map>
#include <vector>

enum class CP : int {
  Default = 1,
  Title = 2,
  Border = 3,
  Highlight = 4,
  Progress = 5,
  ProgressBg = 6,
  Status = 7,
  Artist = 8,
  TimeStamp = 9,
  Inactive = 10,
};

struct ThemeColor {
  short fg = COLOR_WHITE;
  short bg = COLOR_BLACK;
};

struct Theme {
  std::string name;
  std::unordered_map<int, ThemeColor> colors;
  std::unordered_map<int, attr_t> attrs;
};

class ThemeManager {
public:
  ThemeManager();

  void loadFromDisk();

  bool setTheme(const std::string &name);

  void apply() const;

  int pair(CP cp) const;

  attr_t attr(CP cp) const;

  std::vector<std::string> list() const;

  const std::string &current() const;

  static std::string themesDir();

private:
  std::unordered_map<std::string, Theme> themes_;
  std::string active_;

  static std::string lower(std::string s);
  static std::string trim(const std::string &s);
  static short resolveColor(const std::string &s);
  static attr_t resolveAttr(const std::string &s);

  Theme parse(const std::string &path) const;

  static void ensureDirs();
  static void copyDefaultThemes();

  void registerBuiltins();
};

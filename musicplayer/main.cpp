#include "inc/ui.hpp"
#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
  std::string musicDir;

  if (argc > 1) {
    musicDir = argv[1];
  } else {
    const char *home = getenv("HOME");
    musicDir = std::string(home ? home : ".") + "/Music";
  }

  Player player;
  UI ui(player, musicDir);
  ui.Start();
  return 0;
}

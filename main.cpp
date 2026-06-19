#include "inc/player.hpp"

int main(int argc, char *argv[]) {
  Player player;

  player.SetFilePath("/home/mrcat/Music/02 - Sunflower (Spider-Man_ Into the "
                     "Spider-Verse).mp3");

  player.play();

  return 0;
}

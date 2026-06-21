#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../third_party/miniaudio/miniaudio.h"
#include <string>

enum PlayerModeState { /// NOTE: Mode of how to play music
  // SHUFFLE, /// NOTE: shuffle the music 1 time, then play as normal()
  REPEAT, /// NOTE: Only play 1 music (when u press next, itll go to next music)
  NORMAL, /// NOTE: play as normal
  RANDOM, /// Every time u press next, it'll select a random music(u can go
          /// previus music only for 5 times bcz itll stored as a vector)
};

class Player {
private:
  ma_engine engine;
  ma_sound sound;
  bool soundLoaded = false;
  bool engineOk = false;
  std::string filePath;

public:
  Player();
  ~Player();

  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;

  void Play();
  void Pause();
  void Stop();
  void TogglePause();
  bool IsPlaying();
  bool IsLoaded() const { return soundLoaded; }

  void SetFilePath(const std::string &newFilePath);
  bool LoadCurrent();
  double GetCursorSeconds();
  double GetLengthSeconds();
  void SetVolume(float v);
};

#endif // PLAYER_HPP

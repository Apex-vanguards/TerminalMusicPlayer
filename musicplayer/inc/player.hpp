#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../third_party/miniaudio/miniaudio.h"
#include <string>

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

  // no copying: ma_engine/ma_sound own native resources
  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;

  void Play();
  void Pause();
  void Stop();
  void TogglePause();
  bool IsPlaying();
  bool IsLoaded() const { return soundLoaded; }

  void SetFilePath(const std::string &newFilePath);
  bool LoadCurrent(); // (re)loads `sound` from filePath
  double GetCursorSeconds();
  double GetLengthSeconds();
  void SetVolume(float v);
};

#endif // PLAYER_HPP

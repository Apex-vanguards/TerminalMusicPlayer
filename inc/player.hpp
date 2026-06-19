
#include <miniaudio/miniaudio.h>

class Player {
private:
  ma_engine engine;
  ma_sound sound;

  const char *filePath;

public:
  Player();
  ~Player();

  void play();
  bool IsPlaying();
  void SetFilePath(const char *newfilePath);
  void InitSound();
};

#include <stdexcept>
#include <unistd.h>
#define MINIAUDIO_IMPLEMENTATION
#include "../inc/player.hpp"

Player::Player() : filePath(nullptr) {

  if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
    throw std::runtime_error("Failed");
  }

  ma_sound_init_from_file(&engine, filePath, 0, NULL, NULL, &sound);
}

void Player::InitSound() {
  if (filePath != nullptr)
    ma_sound_init_from_file(&engine, filePath, 0, NULL, NULL, &sound);
}

Player::~Player() { ma_engine_uninit(&engine); }

void Player::play() {
  InitSound();

  ma_sound_start(&sound);

  while (IsPlaying()) {
    usleep(1000000);
  }
}

void Player::SetFilePath(const char *newtfilePath) { filePath = newtfilePath; }

bool Player::IsPlaying() { return !ma_sound_at_end(&sound); }

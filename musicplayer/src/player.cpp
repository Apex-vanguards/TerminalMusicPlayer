#define MINIAUDIO_IMPLEMENTATION
#include "../inc/player.hpp"
#include <stdexcept>

Player::Player() {
  if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
    throw std::runtime_error("Failed to init audio engine");
  }
  engineOk = true;
}

Player::~Player() {
  if (soundLoaded) {
    ma_sound_uninit(&sound);
  }
  if (engineOk) {
    ma_engine_uninit(&engine);
  }
}

void Player::SetFilePath(const std::string &newFilePath) {
  filePath = newFilePath;
}

bool Player::LoadCurrent() {
  if (filePath.empty())
    return false;

  if (soundLoaded) {
    ma_sound_uninit(&sound);
    soundLoaded = false;
  }

  if (ma_sound_init_from_file(&engine, filePath.c_str(), 0, NULL, NULL,
                              &sound) != MA_SUCCESS) {
    return false;
  }
  soundLoaded = true;
  return true;
}

void Player::Play() {
  if (!soundLoaded) {
    if (!LoadCurrent())
      return;
  }
  ma_sound_start(&sound);
}

void Player::Pause() {
  if (soundLoaded)
    ma_sound_stop(&sound);
}

void Player::TogglePause() {
  if (!soundLoaded)
    return;
  if (ma_sound_is_playing(&sound)) {
    ma_sound_stop(&sound);
  } else {
    ma_sound_start(&sound);
  }
}

void Player::Stop() {
  if (soundLoaded) {
    ma_sound_stop(&sound);
    ma_sound_seek_to_pcm_frame(&sound, 0);
  }
}

bool Player::IsPlaying() {
  if (!soundLoaded)
    return false;
  return ma_sound_is_playing(&sound) && !ma_sound_at_end(&sound);
}

double Player::GetCursorSeconds() {
  if (!soundLoaded)
    return 0.0;
  float cursor = 0.0f;
  ma_sound_get_cursor_in_seconds(&sound, &cursor);
  return (double)cursor;
}

double Player::GetLengthSeconds() {
  if (!soundLoaded)
    return 0.0;
  float length = 0.0f;
  ma_sound_get_length_in_seconds(&sound, &length);
  return (double)length;
}

void Player::SetVolume(float v) {
  if (soundLoaded)
    ma_sound_set_volume(&sound, v);
}

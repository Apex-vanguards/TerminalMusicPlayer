#include "../inc/playlist.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void Playlist::Add(const std::string &trackName, const std::string &trackPath) {
  for (const auto &e : entries)
    if (e.path == trackPath)
      return;
  entries.push_back({trackName, trackPath});
}

void Playlist::Remove(int index) {
  if (index < 0 || index >= (int)entries.size())
    return;
  entries.erase(entries.begin() + index);
}

bool Playlist::SaveM3U(const std::string &filePath) const {
  std::ofstream f(filePath);
  if (!f.is_open())
    return false;
  f << "#EXTM3U\n";
  for (const auto &e : entries) {
    f << "#EXTINF:-1," << e.name << "\n";
    f << e.path << "\n";
  }
  return true;
}

bool Playlist::LoadM3U(const std::string &filePath) {
  std::ifstream f(filePath);
  if (!f.is_open())
    return false;
  entries.clear();
  std::string line, pendingName;
  while (std::getline(f, line)) {
    if (line.empty())
      continue;
    if (line.rfind("#EXTM3U", 0) == 0)
      continue;
    if (line.rfind("#EXTINF", 0) == 0) {
      auto comma = line.find(',');
      pendingName = (comma != std::string::npos) ? line.substr(comma + 1) : "";
      continue;
    }
    if (line[0] == '#')
      continue;
    PlaylistEntry e;
    e.path = line;
    e.name =
        pendingName.empty() ? fs::path(line).filename().string() : pendingName;
    pendingName.clear();
    entries.push_back(e);
  }
  return true;
}

void Playlist::Clear() { entries.clear(); }

std::string PlaylistManager::PlaylistsDir() {
  const char *h = std::getenv("HOME");
  return std::string(h ? h : "/root") + "/.config/CatMusicPlayer/playlists";
}

std::string PlaylistManager::LastUsedFile() {
  return PlaylistsDir() + "/.last";
}

void PlaylistManager::EnsureDir() { fs::create_directories(PlaylistsDir()); }

void PlaylistManager::SaveLastUsed() const {
  std::ofstream f(LastUsedFile());
  if (f.is_open())
    f << active_.name << "\n";
}

void PlaylistManager::LoadLastUsed() {
  std::ifstream f(LastUsedFile());
  if (!f.is_open())
    return;
  std::string lastName;
  if (!std::getline(f, lastName) || lastName.empty())
    return;

  std::string safeName = lastName;
  for (char &c : safeName)
    if (c == ' ' || c == '/' || c == '\\')
      c = '_';

  std::string path = PlaylistsDir() + "/" + safeName + ".m3u";
  if (fs::exists(path)) {
    active_.name = lastName;
    active_.LoadM3U(path);
  }
}

PlaylistManager::PlaylistManager() {
  active_.name = "New Playlist";
  LoadLastUsed();
}

void PlaylistManager::NewPlaylist(const std::string &name) {
  active_.Clear();
  active_.name = name.empty() ? "New Playlist" : name;
}

void PlaylistManager::AddToActive(const std::string &trackName,
                                  const std::string &trackPath) {
  active_.Add(trackName, trackPath);
}

void PlaylistManager::RemoveFromActive(int index) { active_.Remove(index); }

bool PlaylistManager::SaveActive() {
  if (active_.name.empty())
    active_.name = "playlist";
  EnsureDir();

  std::string safeName = active_.name;
  for (char &c : safeName)
    if (c == ' ' || c == '/' || c == '\\')
      c = '_';

  std::string path = PlaylistsDir() + "/" + safeName + ".m3u";
  bool ok = active_.SaveM3U(path);
  if (ok)

    SaveLastUsed();
  return ok;
}

bool PlaylistManager::LoadPlaylist(const std::string &filePath) {
  active_.Clear();
  active_.name = fs::path(filePath).stem().string();
  bool ok = active_.LoadM3U(filePath);
  if (ok)
    SaveLastUsed();
  return ok;
}

std::vector<std::string> PlaylistManager::ListSavedPlaylists() const {
  std::vector<std::string> result;
  std::string dir = PlaylistsDir();
  if (!fs::exists(dir))
    return result;
  for (const auto &entry : fs::directory_iterator(dir))
    if (entry.path().extension() == ".m3u")
      result.push_back(entry.path().string());
  return result;
}

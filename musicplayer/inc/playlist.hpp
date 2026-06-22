#pragma once
#include <string>
#include <vector>

struct PlaylistEntry {
  std::string name;
  std::string path;
};

class Playlist {
public:
  std::string name;
  std::vector<PlaylistEntry> entries;

  void Add(const std::string &trackName, const std::string &trackPath);
  void Remove(int index);
  bool SaveM3U(const std::string &filePath) const;
  bool LoadM3U(const std::string &filePath);
  void Clear();
  bool IsEmpty() const { return entries.empty(); }
  int Size() const { return (int)entries.size(); }
};

class PlaylistManager {
public:
  PlaylistManager();
  ~PlaylistManager() { SaveActive(); }

  void NewPlaylist(const std::string &name);
  void AddToActive(const std::string &trackName, const std::string &trackPath);
  void RemoveFromActive(int index);
  bool SaveActive();
  bool LoadPlaylist(const std::string &filePath);

  std::vector<std::string> ListSavedPlaylists() const;

  Playlist &Active() { return active_; }
  const Playlist &Active() const { return active_; }

  static std::string PlaylistsDir();

private:
  Playlist active_;
  static void EnsureDir();
  static std::string LastUsedFile();
  void SaveLastUsed() const;
  void LoadLastUsed();
};

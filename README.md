# 🎵 Terminal Music Player

A lightweight, terminal-based music player written in C++ for Linux. Play your audio files directly from the command line with a clean ncurses interface — no GUI needed.

## Features

- Terminal UI powered by ncurses
- Audio playback via ALSA
- Lightweight and fast — pure C++
- Desktop entry included for easy launching
- AUR-ready with a PKGBUILD for Arch Linux users

## Requirements

- `gcc` (C++ compiler)
- `cmake`
- `alsa-lib`
- `ncurses`

These dependencies are automatically installed by the install script on Arch-based systems.

## Installation

### Manual Installation

```bash
git clone https://github.com/Apex-vanguards/TerminalMusicPlayer.git
cd TerminalMusicPlayer
make -C musicplayer
sudo ./install.sh
```

The install script will:

- Install the `musicplayer` binary to `/usr/local/bin/`
- Copy the icon to `/usr/share/pixmaps/`
- Create a `.desktop` entry so it appears in your application launcher

## Usage

After installation, launch from the terminal:

```bash
musicplayer
```

Or find **Cat Music Player** in your desktop application menu (under AudioVideo).

## Project Structure

```
TerminalMusicPlayer/
├── musicplayer/       # C++ source code and Makefile
├── install.sh         # Installation script (requires root)
├── icon.ico           # Application icon
└── LISENSE            # MIT License
```

## License

This project is licensed under the MIT License — see the [LISENSE](LISENSE) file for details.

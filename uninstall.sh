#!/bin/bash
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (use sudo)"
  exit
fi

APP_NAME="CatMusicPlayer"
INSTALL_DIR="/usr/local/bin"
ICON_PATH="/usr/share/pixmaps/CatMusicPlayer.ico"
DESKTOP_FILE="/usr/share/applications/musicplayer.desktop"
DEPENDENCIES=("cmake" "gcc" "alsa-lib" "ncurses")
THEMES_DIR="/usr/share/CatMusicPlayer/themes"

echo "Started"

rm -rf $INSTALL_DIR/musicplayer
rm -rf $DESKTOP_FILE
rm -rf $ICON_PATH
rm -rf $THEMES_DIR/*

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

echo "--- Installation started ---"

echo "Checking and installing dependencies..."
pacman -S --needed --noconfirm "${DEPENDENCIES[@]}"

echo "Installing binary files..."
cp ./musicplayer/musicplayer $INSTALL_DIR/
chmod +x $INSTALL_DIR/musicplayer

echo "Installing icon..."
cp icon.ico $ICON_PATH

echo "Creating desktop entry..."
cat <<EOF >$DESKTOP_FILE
[Desktop Entry]
Name=Cat Music Player
Exec=$INSTALL_DIR/musicplayer
Icon=$ICON_PATH
Type=Application
Terminal=true
Categories=AudioVideo;Player;
EOF

echo "Installation completed successfully!"

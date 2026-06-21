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

echo "--- Installation started ---"

echo "Checking and installing dependencies..."
pacman -S --needed --noconfirm "${DEPENDENCIES[@]}"

echo "Installing binary files..."
rm $INSTALL_DIR/musicplayer
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

echo "Creating themes folder"
mkdir -p /usr/share/CatMusicPlayer/themes
cp -r themes/* /usr/share/CatMusicPlayer/themes

echo "Installation completed successfully!"

#!/bin/bash

: "${pkgdir:=/}"

INSTALL_DIR="$pkgdir/usr/local/bin"
ICON_PATH="$pkgdir/usr/share/pixmaps/CatMusicPlayer.ico"
DESKTOP_FILE="$pkgdir/usr/share/applications/musicplayer.desktop"
THEMES_DIR="$pkgdir/usr/share/CatMusicPlayer/themes"

mkdir -p "$INSTALL_DIR"
mkdir -p "$(dirname "$ICON_PATH")"
mkdir -p "$(dirname "$DESKTOP_FILE")"
mkdir -p "$THEMES_DIR"

cp ./musicplayer/musicplayer "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/musicplayer"

cp icon.ico "$ICON_PATH"

cat <<EOF >"$DESKTOP_FILE"
[Desktop Entry]
Name=Cat Music Player
Exec=/usr/local/bin/musicplayer
Icon=/usr/share/pixmaps/CatMusicPlayer.ico
Type=Application
Terminal=true
Categories=AudioVideo;Player;
EOF

cp -r themes/* "$THEMES_DIR/"

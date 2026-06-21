pkgname=terminalmusicplayer-git
pkgver=1.0.0
pkgrel=1
pkgdesc="Terminal tabanlı müzik çalar"
arch=('x86_64')
url="https://github.com/Apex-vanguards/TerminalMusicPlayer"
license=('MIT')
source=("git+https://github.com/Apex-vanguards/TerminalMusicPlayer.git")
sha256sums=('SKIP')

build() {
  cd "TerminalMusicPlayer"
  make -C musicplayer
}

package() {
  cd "TerminalMusicPlayer"
  DESTDIR="$pkgdir" ./install.sh
}

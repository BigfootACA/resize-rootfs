# Maintainer: BigfootACA <bigfoot@classfun.cn>

pkgname=resize-rootfs
pkgver=0.1.0
pkgrel=1
pkgdesc="Resize rootfs partition and filesystem to fill disk"
arch=(i686 x86_64 arm armv6h armv7h aarch64)
url="https://github.com/BigfootACA/resize-rootfs"
license=(GPL-3.0-or-later)
depends=(
	glibc
	systemd
	gcc-libs
	util-linux-libs
	fmt
)
makedepends=(
	meson
)
optdepends=(
	"e2fsprogs: for ext2/ext3/ext4 filesystem resize"
	"btrfs-progs: for btrfs filesystem resize"
	"xfsprogs: for xfs filesystem resize"
)
source=(PKGBUILD)
sha256sums=(SKIP)

build() {
	dir="$(dirname "$(realpath "$srcdir/PKGBUILD")")"
	arch-meson "$dir" build
	meson compile -C build
}

package() {
	meson install -C build --destdir "$pkgdir"
}

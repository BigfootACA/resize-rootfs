# Maintainer: BigfootACA <bigfoot@classfun.cn>

pkgname=resize-rootfs
pkgver=0.0.1
pkgrel=1
pkgdesc="Resize rootfs partition and filesystem to fill disk"
arch=(any)
url="https://github.com/BigfootACA/resize-rootfs"
license=(GPL-3.0-or-later)
depends=(
	bash
	systemd
        coreutils
        util-linux
        gawk
        parted
        expect
        jq
)
optdepends=(
        "e2fsprogs: for ext2/ext3/ext4 filesystem resize"
        "btrfs-progs: for btrfs filesystem resize"
        "xfsprogs: for xfs filesystem resize"
)
source=(
	resize.sh
        resize-rootfs.service
)
md5sums=('eb643727a6fe038d844754e57ac97c98'
         '66daaea89e11d60f5c922d0b6ef6a89f')
sha256sums=('dde7a1b91fa791f1129f95c90e41fb0f3d62228a0d91e0b9593f4b5d64c54cc8'
            '430fcc2d041284d19d70fc40547ff30791a164a7d0ec62df16c416828e9ed527')
sha512sums=('b55cece63de35d7afe3ea2fb484df083f1d41757063931edaa9a8a4ea91ec0dee01b997611046badca017cda2da58f0ca2a1b9059f29d0908085414e1d1deaf9'
            '882e982786d5a71a91d03927648452be991e5dd1a0175b80b7802101134409570e37d905e274cd66ab39dff0a0f2bd6fa1e0504c99bd63fe13f525caecf0c594')

package() {
	install -vDm755 "$srcdir/resize.sh" "$pkgdir/usr/bin/resize-rootfs"
	install -vDm644 "$srcdir/resize-rootfs.service" "$pkgdir/usr/lib/systemd/system/resize-rootfs.service"
}

#!/bin/bash
set -ex
builddir=/tmp/build-resize-rootfs
file=/tmp/test.img
mnt=/tmp/root
umount "$mnt" || true
losetup -nO NAME,BACK-FILE | \
	awk '$2=="/tmp/test.img"{print $1}' | \
	while read -r dev; do losetup -d "$dev" || true; done
rm -f "$file"
fallocate -l 512M "$file"
parted -s "$file" \
	mklabel gpt \
	mkpart boot fat32 0% 100M \
	mkpart root fat32 100M 200M
fallocate -l 1G "$file"
dev=$(losetup -f -P --show "$file")
mke2fs -t ext4 "${dev}p2"
mkdir -p "$mnt"
mount "${dev}p2" "$mnt"
meson setup --buildtype debug . "$builddir"
ninja -C "$builddir"
parted -s "$dev" unit s print free
"$builddir/resize-rootfs" "$mnt"
parted -s "$dev" unit s print free
umount "$mnt" || true
losetup -d "$dev" || true
rm -f "$file"

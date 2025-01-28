#!/bin/bash
###
### Copyright (C) 2025 BigfootACA <bigfoot@classfun.cn>
###
### SPDX-License-Identifier: GPL-3.0-or-later
###
### Root partition resizer
###
### Dependency:
###   ArchLinux: pacman --needed -S bash coreutils util-linux gawk parted expect jq e2fsprogs btrfs-progs xfsprogs
###   Debian: apt install bash coreutils util-linux gawk parted expect jq e2fsprogs btrfs-progs xfsprogs
###

set -e
set -o pipefail

ALLOWED_FSTYPE=(ext2 ext3 ext4 btrfs xfs)
ALLOWED_LABEL=(gpt msdos)
export LANG=C LANGUAGE=C LC_ALL=C

# Manual specified target
if [ -n "$1" ]; then
        if [ -d "$1" ]; then
                TARGET_MNT="$1"
        elif [ -b "$1" ]; then
                TARGET_BLOCK="$1"
        else
                echo "Bad target $1" >&2
                exit 1
        fi
fi

# Find mountpoint
if [ -z "${TARGET_MNT}" ]; then
        if [ -z "${TARGET_BLOCK}" ]; then
                TARGET_MNT=/
        else
                TARGET_MNT="$(findmnt --noheadings --nofsroot --first-only --output TARGET "${TARGET_BLOCK}")"
        fi
fi
if ! mountpoint -q "${TARGET_MNT}" 2>/dev/null; then
        echo "Target folder ${TARGET_MNT} is not a mountpoint" >&2
        exit 1
fi
echo "Target mountpoint: ${TARGET_MNT}"

# Find target device block
if [ -z "${TARGET_BLOCK}" ]; then
        TARGET_BLOCK="$(findmnt --noheadings --nofsroot --first-only --output SOURCE "${TARGET_MNT}")"
fi
if ! [ -b "${TARGET_BLOCK}" ]; then
        echo "Target ${TARGET_BLOCK} is not a block device" >&2
        exit 1
fi
echo "Target block: ${TARGET_BLOCK}"

# Get filesystem type
TARGET_FSTYPE="$(blkid --output value --match-tag TYPE "${TARGET_BLOCK}")"
echo "Target filesystem type: ${TARGET_FSTYPE}"

ALLOWED=false
for FS in "${ALLOWED_FSTYPE[@]}"; do
        if [ "$FS" == "${TARGET_FSTYPE}" ]; then
                ALLOWED=true
                break
        fi
done
if ! "$ALLOWED"; then
        echo "Filesystem type ${TARGET_FSTYPE} unsupported" >&2
        exit 1
fi

# Find target device parent disk
TARGET_DISK="/dev/$(lsblk --output PKNAME --raw --noheadings "${TARGET_BLOCK}")"
if ! [ -b "${TARGET_DISK}" ]; then
        echo "Target ${TARGET_DISK} parent disk is not a block device" >&2
        exit 1
fi
echo "Target parent disk: ${TARGET_DISK}"
parted --fix --script "${TARGET_DISK}" print free 0</dev/null

# Get target disk layout label
TARGET_LAYOUT="$(parted --fix --json --script "${TARGET_DISK}" print | jq -r .disk.label)"
ALLOWED=false
for LABEL in "${ALLOWED_LABEL[@]}"; do
        if [ "$LABEL" == "${TARGET_LAYOUT}" ]; then
                ALLOWED=true
                break
        fi
done
if ! "$ALLOWED"; then
        echo "Disk layout ${TARGET_LAYOUT} unsupported" >&2
        exit 1
fi
echo "Target disk layout: ${TARGET_LAYOUT}"

# Get target device partition number
TARGET_NUM="$(lsblk --output PARTN --raw --noheadings "${TARGET_BLOCK}")"
echo "Target partition number: ${TARGET_NUM}"

# Get free area after partition
RESULT="$(parted --fix --json --script "${TARGET_DISK}" unit s print free | \
        jq -r '.disk.partitions | . as $parts | to_entries[] |
        select(.value.number == '"${TARGET_NUM}"') |
        .key + 1 | $parts[.] | select(.type == "free") |
        (.end | rtrimstr("s")) + " " + (.size | rtrimstr("s"))
')"
if [ -z "$RESULT" ]; then
        echo "No free area found"
        exit 0
fi
read -r TARGET_FREE_END TARGET_FREE_SIZE <<< "$RESULT"
echo "Available sectors in disk: ${TARGET_FREE_SIZE}"
if [ "${TARGET_FREE_SIZE}" -le 2048 ]; then
        echo "Partition is already filled"
        exit 0
fi

# Resize partition
echo "Starting resize partition..."
expect << EOF
set timeout 3
spawn parted "${TARGET_DISK}" unit s resizepart "${TARGET_NUM}" "${TARGET_FREE_END}"
expect -re "Warning: Partition .* is being used. Are you sure you want to continue?" {
        expect "Yes/No?" {
                send "Yes\r"
        }
}
expect eof
exit [lindex [wait] 3]
EOF
parted --fix --script "${TARGET_DISK}" print free 0</dev/null

# Resize filesystem
case "${TARGET_FSTYPE}" in
        ext2|ext3|ext4)
                resize2fs "${TARGET_BLOCK}"
        ;;
        xfs)
                xfs_growfs "${TARGET_MNT}"
        ;;
        btrfs)
                btrfs filesystem resize max "${TARGET_MNT}"
        ;;
esac
echo "Done"

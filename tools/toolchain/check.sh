#!/usr/bin/env sh
set -eu

status=0

check_required() {
    name="$1"
    if command -v "$name" >/dev/null 2>&1; then
        echo "ok: $name"
    else
        echo "missing: $name"
        status=1
    fi
}

check_optional() {
    name="$1"
    note="$2"
    if command -v "$name" >/dev/null 2>&1; then
        echo "ok: $name"
    else
        echo "missing: $name ($note)"
    fi
}

if command -v i686-elf-gcc >/dev/null 2>&1; then
    echo "ok: i686-elf-gcc"
else
    check_required gcc
fi

check_required make
check_optional qemu-system-i386 "required for QEMU boot tests"
check_optional grub-mkrescue "required for bootable ISO image"
check_optional xorriso "required by grub-mkrescue"
check_optional mformat "install mtools for grub-mkrescue"
check_optional grub-file "required for Multiboot verification"

exit "$status"

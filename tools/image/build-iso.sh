#!/usr/bin/env sh
set -eu

kernel_elf="${1:-build/hnmos-kernel01.elf}"
iso_image="${2:-build/hnmos-kernel01.iso}"
grub_cfg="${3:-tools/image/grub.cfg}"
iso_root="${4:-build/iso-root}"

for tool in grub-mkrescue xorriso mformat; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "missing: $tool" >&2
        exit 1
    fi
done

mkdir -p "$iso_root/boot/grub"
cp "$kernel_elf" "$iso_root/boot/hnmos-kernel01.elf"
cp "$grub_cfg" "$iso_root/boot/grub/grub.cfg"

grub-mkrescue -o "$iso_image" "$iso_root"

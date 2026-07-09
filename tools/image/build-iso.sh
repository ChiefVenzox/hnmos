#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
kernel_elf="${1:-$repo_root/build/hnmos-kernel01.elf}"
iso_image="${2:-$repo_root/build/hnmos-kernel01.iso}"
grub_cfg="${3:-$repo_root/tools/image/grub.cfg}"
iso_root="${4:-$repo_root/build/iso-root}"
staging_dir=

cleanup() {
    if [ -n "$staging_dir" ]; then
        rm -rf "$staging_dir"
    fi
}
trap cleanup 0 1 2 15

for tool in grub-mkrescue xorriso mformat; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "missing: $tool" >&2
        exit 1
    fi
done

case "$iso_root" in
    "" | /)
        echo "invalid ISO staging directory: $iso_root" >&2
        exit 1
        ;;
esac

mkdir -p "$iso_root" "$(dirname -- "$iso_image")"
staging_dir=$(mktemp -d "$iso_root/hnmos-stage.XXXXXX")
mkdir -p "$staging_dir/boot/grub"
cp "$kernel_elf" "$staging_dir/boot/hnmos-kernel01.elf"
cp "$grub_cfg" "$staging_dir/boot/grub/grub.cfg"

grub-mkrescue -o "$iso_image" "$staging_dir"

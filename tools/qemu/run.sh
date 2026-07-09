#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
mode="${1:-kernel}"
artifact="${2:-}"

case "$mode" in
    kernel)
        : "${artifact:=$repo_root/build/hnmos-kernel01.elf}"
        ;;
    image | iso)
        : "${artifact:=$repo_root/build/hnmos-kernel01.iso}"
        ;;
    *)
        echo "usage: tools/qemu/run.sh [kernel|image] [artifact]" >&2
        exit 2
        ;;
esac

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "missing: qemu-system-i386" >&2
    exit 1
fi

case "$mode" in
    kernel)
        exec qemu-system-i386 \
            -machine pc \
            -m 128M \
            -kernel "$artifact" \
            -serial stdio \
            -no-reboot \
            -no-shutdown
        ;;
    image | iso)
        exec qemu-system-i386 \
            -machine pc \
            -m 128M \
            -cdrom "$artifact" \
            -boot d \
            -serial stdio \
            -no-reboot \
            -no-shutdown
        ;;
esac

#!/usr/bin/env sh
set -eu

mode="${1:-kernel}"
artifact="${2:-}"

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "missing: qemu-system-i386" >&2
    exit 1
fi

case "$mode" in
    kernel)
        : "${artifact:=build/hnmos-kernel01.elf}"
        exec qemu-system-i386 \
            -machine pc \
            -m 128M \
            -kernel "$artifact" \
            -serial stdio \
            -no-reboot \
            -no-shutdown
        ;;
    image | iso)
        : "${artifact:=build/hnmos-kernel01.iso}"
        exec qemu-system-i386 \
            -machine pc \
            -m 128M \
            -cdrom "$artifact" \
            -boot d \
            -serial stdio \
            -no-reboot \
            -no-shutdown
        ;;
    *)
        echo "usage: tools/qemu/run.sh [kernel|image] [artifact]" >&2
        exit 2
        ;;
esac

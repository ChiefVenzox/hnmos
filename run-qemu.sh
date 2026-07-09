#!/usr/bin/env sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mode="${1:-kernel}"

case "$mode" in
    kernel)
        make -C "$repo_root" kernel
        exec "$repo_root/tools/qemu/run.sh" kernel "$repo_root/build/hnmos-kernel01.elf"
        ;;
    image | iso)
        make -C "$repo_root" image
        exec "$repo_root/tools/qemu/run.sh" image "$repo_root/build/hnmos-kernel01.iso"
        ;;
    *)
        echo "usage: ./run-qemu.sh [kernel|image]" >&2
        exit 2
        ;;
esac

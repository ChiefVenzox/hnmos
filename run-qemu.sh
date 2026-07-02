#!/usr/bin/env sh
set -eu

mode="${1:-kernel}"

case "$mode" in
    kernel)
        make kernel
        exec ./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
        ;;
    image | iso)
        make image
        exec ./tools/qemu/run.sh image build/hnmos-kernel01.iso
        ;;
    *)
        echo "usage: ./run-qemu.sh [kernel|image]" >&2
        exit 2
        ;;
esac

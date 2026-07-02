#!/usr/bin/env sh
set -eu

target="${1:-kernel}"

case "$target" in
    kernel)
        exec make kernel
        ;;
    image | iso)
        exec make image
        ;;
    verify)
        exec make verify
        ;;
    tools)
        exec make check-tools
        ;;
    clean)
        exec make clean
        ;;
    *)
        echo "usage: ./build.sh [kernel|image|verify|tools|clean]" >&2
        exit 2
        ;;
esac

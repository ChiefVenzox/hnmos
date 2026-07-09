#!/usr/bin/env sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
target="${1:-kernel}"

case "$target" in
    kernel)
        exec make -C "$repo_root" kernel
        ;;
    image | iso)
        exec make -C "$repo_root" image
        ;;
    verify)
        exec make -C "$repo_root" verify
        ;;
    check)
        exec make -C "$repo_root" check
        ;;
    tools)
        exec make -C "$repo_root" check-tools
        ;;
    clean)
        exec make -C "$repo_root" clean
        ;;
    *)
        echo "usage: ./build.sh [kernel|image|verify|check|tools|clean]" >&2
        exit 2
        ;;
esac

#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)

echo "HNMos v0.0.1 demo"
echo "target: bare-metal Kernel 01 on QEMU"
echo

echo "== build =="
"$repo_root/build.sh"

echo
echo "== verify =="
"$repo_root/build.sh" verify

echo
if command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "== qemu =="
    exec "$repo_root/run-qemu.sh"
else
    echo "skip: qemu-system-i386 not found"
    echo "install QEMU, then run:"
    echo "  $repo_root/run-qemu.sh"
fi

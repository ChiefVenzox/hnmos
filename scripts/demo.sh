#!/usr/bin/env sh
set -eu

echo "HNMos v0.0.1 demo"
echo "target: bare-metal Kernel 01 on QEMU"
echo

echo "== build =="
./build.sh

echo
echo "== verify =="
./build.sh verify

echo
if command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "== qemu =="
    exec ./run-qemu.sh
else
    echo "skip: qemu-system-i386 not found"
    echo "install QEMU, then run:"
    echo "  ./run-qemu.sh"
fi

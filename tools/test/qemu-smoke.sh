#!/usr/bin/env sh
set -eu

artifact="${1:-build/hnmos-kernel01.elf}"
log_file="${TMPDIR:-/tmp}/hnmos-qemu-smoke.log"

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    echo "skip: qemu-system-i386 not found"
    exit 0
fi

if ! command -v timeout >/dev/null 2>&1; then
    echo "missing: timeout"
    exit 1
fi

if [ ! -f "$artifact" ]; then
    make kernel
fi

rm -f "$log_file"

set +e
timeout 8s qemu-system-i386 \
    -machine pc \
    -m 128M \
    -kernel "$artifact" \
    -display none \
    -serial stdio \
    -no-reboot \
    -no-shutdown \
    >"$log_file" 2>&1
status="$?"
set -e

if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    cat "$log_file"
    echo "qemu smoke failed: qemu exited with status $status"
    exit "$status"
fi

grep -F "HNMos Kernel 01 initialized." "$log_file" >/dev/null
grep -F "AI-native operating layer will be loaded later." "$log_file" >/dev/null
grep -F "HNMos v0.0.1 demo boot." "$log_file" >/dev/null
grep -F "console: minimal output console ready." "$log_file" >/dev/null
grep -F "hnshell: offline demo stub ready." "$log_file" >/dev/null
grep -F "ai-runtime: OS interface stub ready; no model loaded." "$log_file" >/dev/null
grep -F "status: Kernel 01 is idle." "$log_file" >/dev/null

echo "qemu smoke: ok"

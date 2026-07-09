#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
artifact="${1:-$repo_root/build/hnmos-kernel01.elf}"
timeout_seconds="${HNM_QEMU_TIMEOUT_SECONDS:-8}"
runtime_dir=
log_file=
timeout_marker=
qemu_pid=
watchdog_pid=

case "$artifact" in
    /*) ;;
    *)
        if [ ! -f "$artifact" ] && [ -f "$repo_root/$artifact" ]; then
            artifact="$repo_root/$artifact"
        fi
        ;;
esac

case "${HNM_QEMU_REQUIRED:-0}" in
    1 | true | TRUE) qemu_required=1 ;;
    0 | false | FALSE) qemu_required=0 ;;
    *)
        echo "invalid HNM_QEMU_REQUIRED value: ${HNM_QEMU_REQUIRED}" >&2
        exit 2
        ;;
esac

case "${CI:-}" in
    1 | true | TRUE) qemu_required=1 ;;
esac

case "$timeout_seconds" in
    "" | 0 | *[!0-9]*)
        echo "invalid HNM_QEMU_TIMEOUT_SECONDS value: $timeout_seconds" >&2
        exit 2
        ;;
esac

if ! command -v qemu-system-i386 >/dev/null 2>&1; then
    if [ "$qemu_required" -eq 1 ]; then
        echo "missing: qemu-system-i386" >&2
        exit 1
    fi

    echo "skip: qemu-system-i386 not found"
    exit 0
fi

if [ ! -f "$artifact" ]; then
    make -C "$repo_root" kernel
fi

cleanup() {
    if [ -n "$watchdog_pid" ]; then
        kill "$watchdog_pid" >/dev/null 2>&1 || true
    fi
    if [ -n "$qemu_pid" ]; then
        kill "$qemu_pid" >/dev/null 2>&1 || true
    fi
    if [ -n "$runtime_dir" ]; then
        rm -rf "$runtime_dir"
    fi
}
trap cleanup 0 1 2 15

runtime_dir=$(mktemp -d "${TMPDIR:-/tmp}/hnmos-qemu-smoke.XXXXXX")
log_file="$runtime_dir/qemu.log"
timeout_marker="$runtime_dir/timeout"

qemu-system-i386 \
    -machine pc \
    -m 128M \
    -kernel "$artifact" \
    -display none \
    -serial stdio \
    -no-reboot \
    -no-shutdown \
    >"$log_file" 2>&1 &
qemu_pid=$!

(
    sleep "$timeout_seconds"
    if kill -0 "$qemu_pid" >/dev/null 2>&1; then
        : >"$timeout_marker"
        kill "$qemu_pid" >/dev/null 2>&1 || true
    fi
) &
watchdog_pid=$!

set +e
wait "$qemu_pid"
qemu_status=$?
kill "$watchdog_pid" >/dev/null 2>&1
wait "$watchdog_pid" >/dev/null 2>&1
set -e
qemu_pid=
watchdog_pid=

if [ ! -f "$timeout_marker" ] && [ "$qemu_status" -ne 0 ]; then
    cat "$log_file"
    echo "qemu smoke failed: qemu exited with status $qemu_status" >&2
    exit "$qemu_status"
fi

expected_lines='HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
HNMos v0.0.1 demo boot.
sync: cpu/ram/framebuffer checkpoint layer ready.
ai bridge: provider active: HNMos Stub Provider
status: Kernel 01 is idle.
console: minimal output console ready.
hnshell: offline demo stub ready.
ai-runtime: OS interface stub ready; no model loaded.
sync-runtime: cpu/ram/framebuffer checkpoint interface ready.'

printf '%s\n' "$expected_lines" | while IFS= read -r expected_line; do
    if ! grep -F "$expected_line" "$log_file" >/dev/null; then
        cat "$log_file"
        echo "qemu smoke failed: missing log line: $expected_line" >&2
        exit 1
    fi
done

rm -rf "$runtime_dir"
runtime_dir=
trap - 0 1 2 15

echo "qemu smoke: kernel, sync assembly and AI bridge boot path ok"

#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
artifact="${1:-$repo_root/build/hnmos-kernel01.elf}"
socket_path="${TMPDIR:-/tmp}/hnmos-ai-$$.sock"
bridge_pid=

cleanup() {
    if [ -n "$bridge_pid" ]; then
        kill "$bridge_pid" 2>/dev/null || true
        wait "$bridge_pid" 2>/dev/null || true
    fi
    rm -f "$socket_path"
}

trap cleanup 0 1 2 15

for command_name in qemu-system-i386 python3; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "missing: $command_name" >&2
        exit 1
    fi
done

if [ ! -f "$artifact" ]; then
    echo "missing kernel artifact: $artifact" >&2
    exit 1
fi

python3 "$repo_root/tools/ai/openai_bridge.py" --socket "$socket_path" &
bridge_pid=$!

attempt=0
while [ ! -S "$socket_path" ]; do
    if ! kill -0 "$bridge_pid" 2>/dev/null; then
        echo "AI bridge exited before creating its socket" >&2
        exit 1
    fi
    attempt=$((attempt + 1))
    if [ "$attempt" -ge 100 ]; then
        echo "AI bridge socket timeout" >&2
        exit 1
    fi
    sleep 0.05
done

qemu-system-i386 \
    -machine pc \
    -m 128M \
    -kernel "$artifact" \
    -serial stdio \
    -chardev socket,id=hnai,path="$socket_path" \
    -serial chardev:hnai \
    -no-reboot \
    -no-shutdown

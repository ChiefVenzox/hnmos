#!/usr/bin/env sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

make -C "$repo_root" kernel
exec "$repo_root/tools/qemu/run-ai.sh" "$repo_root/build/hnmos-kernel01.elf"

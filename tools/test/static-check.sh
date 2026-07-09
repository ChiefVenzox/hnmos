#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)

echo "== HNMos static check =="
make -C "$repo_root" verify check-shell

if command -v jq >/dev/null 2>&1; then
    echo "== HNMos manifest JSON check =="
    jq empty \
        "$repo_root/sdk/templates/app.manifest.json" \
        "$repo_root/examples/hn-notes/app.manifest.json" \
        "$repo_root/examples/hn-monitor/app.manifest.json" \
        "$repo_root/examples/hn-ai-lab/app.manifest.json"
else
    echo "skip: jq not found; manifest JSON check skipped"
fi

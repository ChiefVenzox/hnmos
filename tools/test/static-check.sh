#!/usr/bin/env sh
set -eu

echo "== HNMos static check =="
make verify

if command -v jq >/dev/null 2>&1; then
    echo "== HNMos manifest JSON check =="
    jq empty \
        sdk/templates/app.manifest.json \
        examples/hn-notes/app.manifest.json \
        examples/hn-monitor/app.manifest.json \
        examples/hn-ai-lab/app.manifest.json
else
    echo "skip: jq not found; manifest JSON check skipped"
fi

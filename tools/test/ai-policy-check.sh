#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
cc_command=${CC_HOST:-cc}
check_dir=$(mktemp -d "${TMPDIR:-/tmp}/hnmos-ai-policy.XXXXXX")
check_binary="$check_dir/ai-policy-check"

trap 'rm -rf "$check_dir"' 0 1 2 15

"$cc_command" \
    -std=c99 -Wall -Wextra -Werror \
    "$repo_root/tests/ai/workspace_policy_test.c" \
    "$repo_root/kernel01/kernel/ai/ai_workspace_policy.c" \
    "$repo_root/kernel01/kernel/ai/ai_session.c" \
    "$repo_root/kernel01/kernel/ai/hnlang_ai_profile.c" \
    "$repo_root/kernel01/kernel/fs/vfs.c" \
    "$repo_root/kernel01/kernel/fs/ramfs.c" \
    -o "$check_binary"

"$check_binary"
echo "ai policy: workspace boundary, RAM draft, and volatile secret lifecycle ok"

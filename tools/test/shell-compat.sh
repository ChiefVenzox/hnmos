#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
script_list=$(mktemp "${TMPDIR:-/tmp}/hnmos-shell-list.XXXXXX")
trap 'rm -f "$script_list"' 0 1 2 15

if ! command -v bash >/dev/null 2>&1; then
    echo "missing: bash (required for dual shell compatibility check)" >&2
    exit 1
fi

find "$repo_root" -type f -name '*.sh' \
    ! -path "$repo_root/build/*" \
    ! -path "$repo_root/.git/*" |
    LC_ALL=C sort >"$script_list"

script_count=0
while IFS= read -r script_path; do
    first_line=$(sed -n '1p' "$script_path")
    if [ "$first_line" != '#!/usr/bin/env sh' ]; then
        echo "shell compatibility failed: non-POSIX shebang: $script_path" >&2
        exit 1
    fi

    sh -n "$script_path"
    bash --posix -n "$script_path"
    if command -v dash >/dev/null 2>&1; then
        dash -n "$script_path"
    fi
    script_count=$((script_count + 1))
done <"$script_list"

for shell_command in sh bash dash; do
    if ! command -v "$shell_command" >/dev/null 2>&1; then
        continue
    fi

    for dispatcher in \
        "$repo_root/build.sh" \
        "$repo_root/run-qemu.sh" \
        "$repo_root/tools/qemu/run.sh"; do
        set +e
        if [ "$shell_command" = bash ]; then
            (cd "${TMPDIR:-/tmp}" && bash --posix "$dispatcher" __hnmos_invalid_command__) \
                >/dev/null 2>&1
        else
            (cd "${TMPDIR:-/tmp}" && "$shell_command" "$dispatcher" __hnmos_invalid_command__) \
                >/dev/null 2>&1
        fi
        dispatcher_status=$?
        set -e

        if [ "$dispatcher_status" -ne 2 ]; then
            echo "shell compatibility failed: $shell_command $dispatcher returned $dispatcher_status" >&2
            exit 1
        fi
    done
done

rm -f "$script_list"
trap - 0 1 2 15

echo "shell compatibility: $script_count scripts pass POSIX sh, Bash and available dash checks"

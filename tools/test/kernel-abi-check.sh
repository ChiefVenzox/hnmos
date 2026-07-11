#!/usr/bin/env sh
set -eu
set -f

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
artifact="${1:-$repo_root/build/hnmos-kernel01.elf}"

case "$artifact" in
    /*) ;;
    *)
        if [ ! -f "$artifact" ] && [ -f "$repo_root/$artifact" ]; then
            artifact="$repo_root/$artifact"
        fi
        ;;
esac

nm_command="${NM:-nm}"
objdump_command="${OBJDUMP:-objdump}"

for command_name in file od "$nm_command" "$objdump_command"; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "missing: $command_name" >&2
        exit 1
    fi
done

if [ ! -f "$artifact" ]; then
    echo "missing kernel artifact: $artifact" >&2
    exit 1
fi

description=$(LC_ALL=C file "$artifact")
case "$description" in
    *"ELF 32-bit"*"executable"*"Intel 80386"*) ;;
    *)
        echo "kernel ABI check failed: expected ELF32 i386" >&2
        echo "$description" >&2
        exit 1
        ;;
esac

set -- $(od -An -N6 -tx1 "$artifact")
if [ "$#" -ne 6 ] ||
    [ "$1" != 7f ] || [ "$2" != 45 ] || [ "$3" != 4c ] || [ "$4" != 46 ] ||
    [ "$5" != 01 ] || [ "$6" != 01 ]; then
    echo "kernel ABI check failed: invalid ELF32 little-endian i386 header" >&2
    exit 1
fi

set -- $(od -An -j16 -N4 -tx1 "$artifact")
if [ "$#" -ne 4 ] ||
    [ "$1" != 02 ] || [ "$2" != 00 ] ||
    [ "$3" != 03 ] || [ "$4" != 00 ]; then
    echo "kernel ABI check failed: expected ET_EXEC for Intel 80386" >&2
    exit 1
fi

entry_address=$(od -An -j24 -N4 -tu4 "$artifact" | awk '{print $1}')
if [ -z "$entry_address" ] || [ "$entry_address" -eq 0 ]; then
    echo "kernel ABI check failed: ELF entry address is zero" >&2
    exit 1
fi

program_header_offset=$(od -An -j28 -N4 -tu4 "$artifact" | awk '{print $1}')
program_header_size=$(od -An -j42 -N2 -tu2 "$artifact" | awk '{print $1}')
program_header_count=$(od -An -j44 -N2 -tu2 "$artifact" | awk '{print $1}')

if [ -z "$program_header_offset" ] || [ -z "$program_header_size" ] ||
    [ -z "$program_header_count" ] || [ "$program_header_size" -lt 32 ] ||
    [ "$program_header_count" -eq 0 ]; then
    echo "kernel ABI check failed: invalid ELF program header table" >&2
    exit 1
fi

entry_is_loadable=0
program_header_index=0
while [ "$program_header_index" -lt "$program_header_count" ]; do
    header_offset=$((program_header_offset + (program_header_index * program_header_size)))
    segment_type=$(od -An -j "$header_offset" -N4 -tu4 "$artifact" | awk '{print $1}')

    if [ "$segment_type" -eq 1 ]; then
        segment_address=$(od -An -j $((header_offset + 8)) -N4 -tu4 "$artifact" | awk '{print $1}')
        segment_size=$(od -An -j $((header_offset + 20)) -N4 -tu4 "$artifact" | awk '{print $1}')
        segment_end=$((segment_address + segment_size))

        if [ "$entry_address" -ge "$segment_address" ] &&
            [ "$entry_address" -lt "$segment_end" ]; then
            entry_is_loadable=1
        fi
    fi

    program_header_index=$((program_header_index + 1))
done

if [ "$entry_is_loadable" -ne 1 ]; then
    echo "kernel ABI check failed: ELF entry is not inside a PT_LOAD segment" >&2
    exit 1
fi

undefined_symbols=$("$nm_command" -u "$artifact")
if [ -n "$undefined_symbols" ]; then
    echo "kernel ABI check failed: undefined symbols" >&2
    printf '%s\n' "$undefined_symbols" >&2
    exit 1
fi

defined_symbol_table=$("$nm_command" -g --defined-only "$artifact")
all_symbol_table=$("$nm_command" -an "$artifact")
start_record=$(printf '%s\n' "$defined_symbol_table" |
    awk '$NF == "_start" { print $1, $(NF - 1); exit }')
set -- $start_record
if [ "$#" -ne 2 ] || [ "$2" != T ] || [ $((0x$1)) -ne "$entry_address" ]; then
    echo "kernel ABI check failed: ELF entry does not match text symbol _start" >&2
    exit 1
fi

stack_bottom_record=$(printf '%s\n' "$all_symbol_table" |
    awk '$NF == "stack_bottom" { print $1, $(NF - 1); exit }')
stack_top_record=$(printf '%s\n' "$all_symbol_table" |
    awk '$NF == "stack_top" { print $1, $(NF - 1); exit }')
set -- $stack_bottom_record
if [ "$#" -ne 2 ] || { [ "$2" != b ] && [ "$2" != B ]; }; then
    echo "kernel ABI check failed: stack_bottom is missing from BSS" >&2
    exit 1
fi
stack_bottom_address=$((0x$1))
set -- $stack_top_record
if [ "$#" -ne 2 ] || { [ "$2" != b ] && [ "$2" != B ]; }; then
    echo "kernel ABI check failed: stack_top is missing from BSS" >&2
    exit 1
fi
stack_top_address=$((0x$1))

if [ $((stack_top_address - stack_bottom_address)) -ne 16384 ] ||
    [ $((stack_top_address & 15)) -ne 0 ]; then
    echo "kernel ABI check failed: boot stack size/alignment is invalid" >&2
    exit 1
fi
stack_top_hex=$(printf '%x' "$stack_top_address")
stack_load_pattern="mov[a-z]*.*\\\$0x0*${stack_top_hex}.*%esp"

required_symbols='hnm_ai_machine_read_eflags
hnm_ai_machine_read_cr0
hnm_sync_cpu_serialize
hnm_sync_memory_barrier
hnm_sync_cpu_pause
hnm_sync_irq_save
hnm_sync_irq_restore
hnm_sync_lock_acquire
hnm_sync_lock_release'

printf '%s\n' "$required_symbols" | while IFS= read -r symbol_name; do
    symbol_type=$(printf '%s\n' "$defined_symbol_table" |
        awk -v name="$symbol_name" '$NF == name { print $(NF - 1); exit }')
    if [ "$symbol_type" != T ]; then
        echo "kernel ABI check failed: missing text assembly symbol $symbol_name" >&2
        exit 1
    fi
done

set -- $(od -An -v -N8192 -tu4 "$artifact")
multiboot_offset=0
multiboot_found=0

while [ "$#" -ge 7 ]; do
    if [ "$1" -eq 464367618 ] && [ "$2" -eq 7 ] &&
        [ "$4" -eq 0 ] && [ "$5" -eq 1024 ] &&
        [ "$6" -eq 768 ] && [ "$7" -eq 32 ]; then
        multiboot_sum=$((($1 + $2 + $3) & 0xffffffff))
        if [ "$multiboot_sum" -eq 0 ]; then
            multiboot_found=1
            break
        fi
    fi

    shift
    multiboot_offset=$((multiboot_offset + 4))
done

if [ "$multiboot_found" -ne 1 ] || [ "$multiboot_offset" -gt 8164 ]; then
    echo "kernel ABI check failed: complete Multiboot video header is not in the first 8192 bytes" >&2
    exit 1
fi

if command -v grub-file >/dev/null 2>&1; then
    grub-file --is-x86-multiboot "$artifact"
fi

check_dir=$(mktemp -d "${TMPDIR:-/tmp}/hnmos-abi-check.XXXXXX")
disassembly_file="$check_dir/disassembly.txt"
function_file="$check_dir/function.txt"
instruction_file="$check_dir/instructions.txt"
trap 'rm -rf "$check_dir"' 0 1 2 15
LC_ALL=C "$objdump_command" -d "$artifact" >"$disassembly_file"

LC_ALL=C awk '
    /^[[:space:]]*[[:xdigit:]]+:/ {
        line = $0
        sub(/^[^:]*:[[:space:]]*/, "", line)
        while (line ~ /^[[:xdigit:]][[:xdigit:]][[:space:]]/) {
            sub(/^[[:xdigit:]][[:xdigit:]][[:space:]]+/, "", line)
        }
        split(line, fields, /[[:space:]]+/)
        if (fields[1] != "") print fields[1]
    }
' "$disassembly_file" >"$instruction_file"

forbidden_register_pattern='%(xmm|ymm|zmm)[0-9]+|%mm[0-7]|%st([[:space:],(]|$)'
forbidden_instruction_pattern='^(f|emms$|ldmxcsr$|stmxcsr$|vzero|xsave|xrstor|lfence$|mfence$|sfence$|movnti|prefetch)'
if grep -E "$forbidden_register_pattern" "$disassembly_file" >/dev/null ||
    grep -E "$forbidden_instruction_pattern" "$instruction_file" >/dev/null; then
    echo "kernel ABI check failed: forbidden SIMD/FPU instruction detected" >&2
    grep -E "$forbidden_register_pattern" "$disassembly_file" | sed -n '1,12p' >&2 || true
    grep -E "$forbidden_instruction_pattern" "$instruction_file" | sed -n '1,12p' >&2 || true
    exit 1
fi

extract_function() {
    function_name="$1"
    LC_ALL=C awk -v label="<$function_name>:" '
        index($0, label) { inside = 1; next }
        inside && /^[[:space:]]*[[:xdigit:]]+[[:space:]]+<[^>]+>:/ { exit }
        inside { print }
    ' "$disassembly_file"
}

assert_function_pattern() {
    function_name="$1"
    pattern="$2"
    extract_function "$function_name" >"$function_file"

    if [ ! -s "$function_file" ] || ! grep -E "$pattern" "$function_file" >/dev/null; then
        echo "kernel ABI check failed: $function_name is missing pattern: $pattern" >&2
        exit 1
    fi
}

assert_function_order() {
    function_name="$1"
    shift
    extract_function "$function_name" >"$function_file"

    if [ ! -s "$function_file" ]; then
        echo "kernel ABI check failed: cannot disassemble $function_name" >&2
        exit 1
    fi

    previous_position=0
    for pattern in "$@"; do
        position=$(grep -n -E "$pattern" "$function_file" | sed -n '1{s/:.*//;p;}')
        if [ -z "$position" ] || [ "$position" -le "$previous_position" ]; then
            echo "kernel ABI check failed: invalid $function_name instruction order at: $pattern" >&2
            exit 1
        fi
        previous_position="$position"
    done
}

return_pattern='[[:space:]]ret[a-z]*[[:space:]]*$'

assert_function_order hnm_ai_machine_read_eflags \
    'pushf' 'pop[a-z]*[[:space:]]+%eax' "$return_pattern"
assert_function_order hnm_ai_machine_read_cr0 \
    '%cr0.*%eax' "$return_pattern"
assert_function_order hnm_sync_cpu_serialize \
    'push[a-z]*[[:space:]]+%ebx' 'xor[a-z]*.*%eax.*%eax' \
    '[[:space:]]cpuid' 'pop[a-z]*[[:space:]]+%ebx' "$return_pattern"
assert_function_pattern hnm_sync_memory_barrier '[[:space:]]lock([[:space:]]|$)'
assert_function_pattern hnm_sync_memory_barrier '[[:space:]]add[a-z]*[[:space:]]'
assert_function_order hnm_sync_cpu_pause '[[:space:]]pause' "$return_pattern"
assert_function_order hnm_sync_irq_save \
    'pushf' 'pop[a-z]*[[:space:]]+%eax' '[[:space:]]cli' "$return_pattern"
assert_function_order hnm_sync_irq_restore \
    'push[a-z]*.*0x4\(%esp\)' 'popf' "$return_pattern"
assert_function_order hnm_sync_lock_acquire \
    'mov[a-z]*.*0x4\(%esp\).*%ecx' 'mov[a-z]*.*\$0x1.*%eax' \
    'xchg[a-z]*.*%eax.*\(%ecx\)' 'test[a-z]*.*%eax.*%eax' \
    '[[:space:]]j(e|z)' '[[:space:]]pause' \
    'cmp[a-z]*.*\$0x0.*\(%ecx\)' '[[:space:]]jne' \
    '[[:space:]]jmp' "$return_pattern"
assert_function_order hnm_sync_lock_release \
    'mov[a-z]*.*0x4\(%esp\).*%eax' 'mov[a-z]*.*\$0x0.*\(%eax\)' "$return_pattern"

assert_function_order _start \
    "$stack_load_pattern" '[[:space:]]cld' 'sub[a-z]*.*\$0x8.*%esp' \
    'push[a-z]*[[:space:]]+%ebx' 'push[a-z]*[[:space:]]+%eax' \
    'call[a-z]*.*<hnm_kernel_main>'
assert_function_order hnm_irq1_stub \
    '[[:space:]]pusha' 'mov[a-z]*.*%esp.*%ebp' \
    'and[a-z]*.*(\$-0x10|\$0xfffffff0).*%esp' '[[:space:]]cld' \
    'call[a-z]*.*<hnm_keyboard_irq_handler>' 'mov[a-z]*.*%ebp.*%esp' \
    '[[:space:]]popa' '[[:space:]]iret'
assert_function_order hnm_irq3_stub \
    '[[:space:]]pusha' 'mov[a-z]*.*%esp.*%ebp' \
    'and[a-z]*.*(\$-0x10|\$0xfffffff0).*%esp' '[[:space:]]cld' \
    'call[a-z]*.*<hnm_ai_serial_irq_handler>' 'mov[a-z]*.*%ebp.*%esp' \
    '[[:space:]]popa' '[[:space:]]iret'
assert_function_order hnm_irq12_stub \
    '[[:space:]]pusha' 'mov[a-z]*.*%esp.*%ebp' \
    'and[a-z]*.*(\$-0x10|\$0xfffffff0).*%esp' '[[:space:]]cld' \
    'call[a-z]*.*<hnm_mouse_irq_handler>' 'mov[a-z]*.*%ebp.*%esp' \
    '[[:space:]]popa' '[[:space:]]iret'

rm -rf "$check_dir"
trap - 0 1 2 15

echo "kernel ABI: ELF32 i686, Multiboot, assembly symbols and no-SIMD policy ok"

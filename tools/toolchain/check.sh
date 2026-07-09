#!/usr/bin/env sh
set -eu
set -f

mode="${1:-all}"
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
status=0

check_required() {
    name="$1"
    if command -v "$name" >/dev/null 2>&1; then
        echo "ok: $name"
    else
        echo "missing: $name"
        status=1
    fi
}

check_optional() {
    name="$1"
    note="$2"
    if command -v "$name" >/dev/null 2>&1; then
        echo "ok: $name"
    else
        echo "missing: $name ($note)"
    fi
}

compiler="${HNM_CC:-}"
arch_flags="${HNM_ARCH_FLAGS:-}"
cpu_flags="${HNM_CPU_FLAGS:--march=i686 -mgeneral-regs-only -mno-mmx -mno-sse -mno-sse2 -mno-avx -msoft-float -mstackrealign}"

if [ -z "$compiler" ]; then
    if command -v i686-elf-gcc >/dev/null 2>&1; then
        compiler=i686-elf-gcc
    elif [ "$(uname -s 2>/dev/null || true)" = Linux ]; then
        case "$(uname -m 2>/dev/null || true)" in
            x86_64 | i386 | i486 | i586 | i686)
                compiler=gcc
                arch_flags=-m32
                ;;
            *)
                compiler=i686-elf-gcc
                ;;
        esac
    else
        compiler=i686-elf-gcc
    fi
fi

compiler_program=${compiler%% *}
check_required "$compiler_program"
check_required file

if [ "$status" -eq 0 ]; then
    probe_dir=$(mktemp -d "${TMPDIR:-/tmp}/hnmos-toolchain.XXXXXX")
    trap 'rm -rf "$probe_dir"' 0 1 2 15

    if ! printf '%s\n' \
        'extern void hnm_probe_asm(void);' \
        'void _start(void) { hnm_probe_asm(); for (;;) { } }' |
        $compiler $arch_flags $cpu_flags \
            -std=gnu99 -ffreestanding -fno-builtin -fno-pic -fno-pie \
            -fno-stack-protector -c -x c -o "$probe_dir/probe.o" -; then
        echo "error: selected compiler cannot emit i686 freestanding C: $compiler"
        status=1
    fi

    if [ "$status" -eq 0 ] && ! printf '%s\n' \
        '.code32' \
        '.section .text' \
        '.global hnm_probe_asm' \
        '.type hnm_probe_asm, @function' \
        'hnm_probe_asm:' \
        '    pushfl' \
        '    popl %eax' \
        '    ret' |
        $compiler $arch_flags -ffreestanding -x assembler-with-cpp \
            -c -o "$probe_dir/probe-asm.o" -; then
        echo "error: selected compiler cannot assemble 32-bit x86 sources: $compiler"
        status=1
    fi

    if [ "$status" -eq 0 ] && ! $compiler $arch_flags \
        -T "$repo_root/linker.ld" -nostdlib -ffreestanding -no-pie \
        -Wl,--build-id=none -Wl,-z,max-page-size=0x1000 \
        "$probe_dir/probe.o" "$probe_dir/probe-asm.o" \
        -o "$probe_dir/probe.elf"; then
        echo "error: selected compiler cannot link an ELF32 i386 kernel: $compiler"
        status=1
    fi

    if [ "$status" -eq 0 ]; then
        description=$(LC_ALL=C file "$probe_dir/probe.elf")
        case "$description" in
            *"ELF 32-bit"*"Intel 80386"*)
                echo "ok: kernel toolchain emits ELF32 i386"
                ;;
            *)
                echo "error: kernel toolchain emitted the wrong format"
                echo "$description"
                status=1
                ;;
        esac
    fi

    rm -rf "$probe_dir"
    trap - 0 1 2 15
fi

if [ "$mode" = all ]; then
    check_required make
    check_optional qemu-system-i386 "required for QEMU boot tests"
    check_optional grub-mkrescue "required for bootable ISO image"
    check_optional xorriso "required by grub-mkrescue"
    check_optional mformat "install mtools for grub-mkrescue"
    check_optional grub-file "required for strict Multiboot verification"
fi

if [ "$status" -ne 0 ]; then
    echo "hint: install i686-elf-gcc or set CROSS_COMPILE=/path/to/i686-elf-" >&2
fi

exit "$status"

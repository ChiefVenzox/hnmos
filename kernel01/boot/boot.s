.set MULTIBOOT_MAGIC, 0x1BADB002
.set MULTIBOOT_ALIGN, 1 << 0
.set MULTIBOOT_MEMINFO, 1 << 1
.set MULTIBOOT_VIDEO_MODE, 1 << 2
.set MULTIBOOT_FLAGS, MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_VIDEO_MODE
.set MULTIBOOT_CHECKSUM, -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

.section .multiboot, "a"
.align 4
.long MULTIBOOT_MAGIC
.long MULTIBOOT_FLAGS
.long MULTIBOOT_CHECKSUM
.long 0
.long 1024
.long 768
.long 32

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp

    push %ebx
    push %eax
    call hnm_kernel_main

.hnm_halt:
    cli
    hlt
    jmp .hnm_halt

.size _start, . - _start

.section .note.GNU-stack, "", @progbits

PROJECT := hnmos
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
ISO_ROOT := $(BUILD_DIR)/iso-root
KERNEL_ELF := $(BUILD_DIR)/hnmos-kernel01.elf
ISO_IMAGE := $(BUILD_DIR)/hnmos-kernel01.iso

BOOT_SRC := kernel01/boot/boot.s
KERNEL_C_SRCS := $(sort \
	$(wildcard kernel01/kernel/*.c) \
	$(wildcard kernel01/kernel/ai/*.c) \
	$(wildcard kernel01/kernel/fs/*.c) \
	$(wildcard kernel01/kernel/graphics/*.c) \
	$(wildcard kernel01/kernel/interface/*.c) \
	$(wildcard kernel01/kernel/interface/screens/*.c) \
	$(wildcard kernel01/kernel/interface/widgets/*.c) \
	$(wildcard kernel01/kernel/memory/*.c) \
	$(wildcard kernel01/kernel/task/*.c) \
	$(wildcard kernel01/interrupts/*.c) \
	$(wildcard kernel01/drivers/*.c) \
	$(wildcard kernel01/console/*.c))
KERNEL_ASM_SRCS := $(sort $(wildcard kernel01/interrupts/*.s))
LINKER_SCRIPT := linker.ld
GRUB_CFG := tools/image/grub.cfg

KERNEL_C_OBJS := $(patsubst kernel01/%.c,$(OBJ_DIR)/%.o,$(KERNEL_C_SRCS))
KERNEL_ASM_OBJS := $(patsubst kernel01/%.s,$(OBJ_DIR)/%.o,$(KERNEL_ASM_SRCS))
KERNEL_OBJS := $(KERNEL_C_OBJS) $(KERNEL_ASM_OBJS)
OBJS := $(OBJ_DIR)/boot.o $(KERNEL_OBJS)

ifneq ($(strip $(CROSS_COMPILE)),)
CC := $(CROSS_COMPILE)gcc
ARCH_FLAGS :=
else ifneq ($(shell command -v i686-elf-gcc 2>/dev/null),)
CC := i686-elf-gcc
ARCH_FLAGS :=
else
CC := gcc
ARCH_FLAGS := -m32
endif

CFLAGS := $(ARCH_FLAGS) -std=gnu99 -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -Wall -Wextra -Werror -O2
ASFLAGS := $(ARCH_FLAGS) -ffreestanding -x assembler-with-cpp
LDFLAGS := $(ARCH_FLAGS) -T $(LINKER_SCRIPT) -nostdlib -ffreestanding -no-pie -Wl,--build-id=none -Wl,-z,max-page-size=0x1000

.PHONY: all kernel image iso run run-kernel clean check-tools check-image-tools verify

all: kernel

kernel: $(KERNEL_ELF)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/boot.o: $(BOOT_SRC) | $(OBJ_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: kernel01/%.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: kernel01/%.s | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

image: $(ISO_IMAGE)

iso: image

$(ISO_IMAGE): check-image-tools $(KERNEL_ELF) $(GRUB_CFG)
	./tools/image/build-iso.sh $(KERNEL_ELF) $(ISO_IMAGE) $(GRUB_CFG) $(ISO_ROOT)

run-kernel: $(KERNEL_ELF)
	./run-qemu.sh kernel

run: $(ISO_IMAGE)
	./run-qemu.sh image

check-tools:
	@./tools/toolchain/check.sh

check-image-tools:
	@command -v grub-mkrescue >/dev/null || { echo "missing: grub-mkrescue"; exit 1; }
	@command -v xorriso >/dev/null || { echo "missing: xorriso"; exit 1; }
	@command -v mformat >/dev/null || { echo "missing: mformat (install mtools)"; exit 1; }

verify: $(KERNEL_ELF)
	@file $(KERNEL_ELF)
	@readelf -h $(KERNEL_ELF) | sed -n '1,18p'
	@if command -v grub-file >/dev/null; then grub-file --is-x86-multiboot $(KERNEL_ELF) && echo "multiboot: ok"; else echo "missing: grub-file"; fi

clean:
	rm -rf $(BUILD_DIR)

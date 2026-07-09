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
	$(wildcard kernel01/kernel/sync/*.c) \
	$(wildcard kernel01/kernel/task/*.c) \
	$(wildcard kernel01/interrupts/*.c) \
	$(wildcard kernel01/drivers/*.c) \
	$(wildcard kernel01/console/*.c))
KERNEL_ASM_SRCS := $(sort \
	$(wildcard kernel01/interrupts/*.s) \
	$(wildcard kernel01/kernel/ai/*.s) \
	$(wildcard kernel01/kernel/sync/*.s))
LINKER_SCRIPT := linker.ld
GRUB_CFG := tools/image/grub.cfg
ISO_BUILDER := tools/image/build-iso.sh

HOST_OS := $(shell uname -s 2>/dev/null || echo unknown)
HOST_ARCH := $(shell uname -m 2>/dev/null || echo unknown)

KERNEL_C_OBJS := $(patsubst kernel01/%.c,$(OBJ_DIR)/%.o,$(KERNEL_C_SRCS))
KERNEL_ASM_OBJS := $(patsubst kernel01/%.s,$(OBJ_DIR)/%.o,$(KERNEL_ASM_SRCS))
KERNEL_OBJS := $(KERNEL_C_OBJS) $(KERNEL_ASM_OBJS)
OBJS := $(OBJ_DIR)/boot.o $(KERNEL_OBJS)

ifneq ($(strip $(CROSS_COMPILE)),)
CC := $(CROSS_COMPILE)gcc
TOOL_PREFIX := $(CROSS_COMPILE)
ARCH_FLAGS :=
else ifneq ($(shell command -v i686-elf-gcc 2>/dev/null),)
CC := i686-elf-gcc
TOOL_PREFIX := i686-elf-
ARCH_FLAGS :=
else ifeq ($(HOST_OS),Linux)
ifneq ($(filter x86_64 i386 i486 i586 i686,$(HOST_ARCH)),)
CC := gcc
TOOL_PREFIX :=
ARCH_FLAGS := -m32
else
CC := i686-elf-gcc
TOOL_PREFIX := i686-elf-
ARCH_FLAGS :=
endif
else
CC := i686-elf-gcc
TOOL_PREFIX := i686-elf-
ARCH_FLAGS :=
endif

NM := $(TOOL_PREFIX)nm
OBJDUMP := $(TOOL_PREFIX)objdump

KERNEL_CPU_FLAGS := -march=i686 -mgeneral-regs-only -mno-mmx -mno-sse -mno-sse2 -mno-avx -msoft-float -mstackrealign
DEPFLAGS := -MMD -MP
CFLAGS := $(ARCH_FLAGS) $(KERNEL_CPU_FLAGS) -std=gnu99 -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -Wall -Wextra -Werror -O2 $(DEPFLAGS)
ASFLAGS := $(ARCH_FLAGS) -ffreestanding -x assembler-with-cpp
LDFLAGS := $(ARCH_FLAGS) -T $(LINKER_SCRIPT) -nostdlib -ffreestanding -no-pie -Wl,--build-id=none -Wl,-z,max-page-size=0x1000

KERNEL_C_DEPS := $(KERNEL_C_OBJS:.o=.d)
BUILD_CONFIG_ID := $(shell printf '%s\n' '$(CC)|$(CFLAGS)|$(ASFLAGS)|$(LDFLAGS)|$(BOOT_SRC)|$(KERNEL_C_SRCS)|$(KERNEL_ASM_SRCS)' | cksum | awk '{print $$1}')
BUILD_CONFIG_STAMP := $(OBJ_DIR)/.build-config-$(BUILD_CONFIG_ID)

.DELETE_ON_ERROR:

.PHONY: all kernel image iso run run-kernel clean check check-tools check-kernel-toolchain check-image-tools check-shell verify

all: kernel

kernel: $(KERNEL_ELF)

check: verify check-shell

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BUILD_CONFIG_STAMP): Makefile | $(OBJ_DIR) check-kernel-toolchain
	@rm -f $(OBJ_DIR)/.build-config-*
	@touch $@

$(OBJ_DIR)/boot.o: $(BOOT_SRC) $(BUILD_CONFIG_STAMP) | $(OBJ_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: kernel01/%.c $(BUILD_CONFIG_STAMP) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: kernel01/%.s $(BUILD_CONFIG_STAMP) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

image: $(ISO_IMAGE)

iso: image

$(ISO_IMAGE): $(KERNEL_ELF) $(GRUB_CFG) $(ISO_BUILDER) | check-image-tools
	./$(ISO_BUILDER) $(KERNEL_ELF) $(ISO_IMAGE) $(GRUB_CFG) $(ISO_ROOT)

run-kernel: $(KERNEL_ELF)
	./tools/qemu/run.sh kernel $(KERNEL_ELF)

run: $(ISO_IMAGE)
	./tools/qemu/run.sh image $(ISO_IMAGE)

check-tools:
	@HNM_CC="$(CC)" HNM_ARCH_FLAGS="$(ARCH_FLAGS)" HNM_CPU_FLAGS="$(KERNEL_CPU_FLAGS)" ./tools/toolchain/check.sh

check-kernel-toolchain:
	@HNM_CC="$(CC)" HNM_ARCH_FLAGS="$(ARCH_FLAGS)" HNM_CPU_FLAGS="$(KERNEL_CPU_FLAGS)" ./tools/toolchain/check.sh kernel

check-image-tools:
	@command -v grub-mkrescue >/dev/null || { echo "missing: grub-mkrescue"; exit 1; }
	@command -v xorriso >/dev/null || { echo "missing: xorriso"; exit 1; }
	@command -v mformat >/dev/null || { echo "missing: mformat (install mtools)"; exit 1; }

check-shell:
	@./tools/test/shell-compat.sh

verify: $(KERNEL_ELF)
	@NM="$(NM)" OBJDUMP="$(OBJDUMP)" ./tools/test/kernel-abi-check.sh $(KERNEL_ELF)

clean:
	rm -rf $(BUILD_DIR)

-include $(KERNEL_C_DEPS)

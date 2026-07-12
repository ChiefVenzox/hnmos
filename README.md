# HNMos

HNMos is an experimental, task-oriented bare-metal operating system built from scratch with an AI-assisted development workflow.

This repository is not a desktop application that runs on Windows, Linux, or macOS. The host computer is used only to edit source code, run the compiler, assembler, and linker, build bootable images, and test the operating system in QEMU. The resulting artifacts are a freestanding kernel ELF and a bootable ISO intended for QEMU today and physical hardware in the future.

## Project Goal

HNMOS-CODEX-14 establishes the first demonstrable HNMos release and the foundation of Kernel 01. The project boots as a real bare-metal system, initializes its own graphics and input stack, exposes a kernel launcher, and provides a controlled AI Studio integration for HNLang development.

## Current Highlights

- Multiboot-compatible 32-bit x86 kernel written in C and assembly.
- 1024x768x32 framebuffer graphics with a kernel-side launcher and UI primitives.
- PS/2 keyboard and mouse input through IRQ-driven event handling.
- Physical memory management, an early heap, a RAM filesystem, and a VFS skeleton.
- HNShell commands and a cooperative task registry.
- AI Studio launcher application with an HNLang editor.
- User-supplied OpenAI API credentials transferred through a host bridge without being compiled into the kernel or committed to the repository.
- A deny-by-default AI policy that limits generated changes to the writable HNLang workspace.
- A compact HNLang AI profile with reusable combinators for efficient program generation.

## Repository Layout

```text
hnmos/
|-- README.md
|-- RELEASE_NOTES.md
|-- Makefile
|-- build.sh
|-- run-qemu.sh
|-- run-qemu-ai.sh
|-- linker.ld
|-- docs/
|-- kernel01/
|   |-- boot/
|   |-- kernel/
|   |   |-- ai/
|   |   |-- graphics/
|   |   |-- interface/
|   |   |-- memory/
|   |   `-- task/
|   |-- console/
|   |-- memory/
|   |-- interrupts/
|   |-- timer/
|   |-- scheduler/
|   |-- syscalls/
|   |-- events/
|   |-- drivers/
|   `-- fs/
|       `-- metadata/
|-- shell/
|   |-- hnshell/
|   |-- parser/
|   |-- commands/
|   |   `-- events/
|   `-- task_router/
|-- ai/
|   |-- runtime_interface/
|   |-- task_engine/
|   |-- context_router/
|   |-- safety_layer/
|   |-- permission_manifest/
|   |-- action_audit/
|   |-- policy_engine/
|   |-- model_runtime_spec/
|   |-- memory_index/
|   |-- semantic_fs/
|   |-- indexer/
|   `-- event_reader/
|-- lang/
|   `-- hnlang/
|       |-- parser/
|       |-- repl/
|       `-- runtime/
|-- ui/
|   |-- console/
|   |-- framebuffer/
|   |-- window_manager/
|   |-- command_palette/
|   |-- assistant_panel/
|   `-- task_monitor/
|-- sdk/
|   |-- templates/
|   `-- examples/
|-- tools/
|   |-- ai/
|   |-- toolchain/
|   |-- qemu/
|   |-- image/
|   `-- test/
|-- examples/
|   |-- hnlang/
|   |-- hn-notes/
|   |-- hn-monitor/
|   |-- hn-ai-lab/
|   `-- demo_task.hn
|-- scripts/
|   `-- demo.sh
|-- tests/
|   |-- kernel/
|   |-- shell/
|   |-- ai/
|   |-- hnlang/
|   `-- knowledge_fs/
`-- .github/
    `-- workflows/
```

## Directory Guide

- `docs/` contains architecture, build, boot image, subsystem, and contribution documentation.
- `kernel01/` contains the first bootable HNMos kernel.
- `kernel01/boot/` contains the Multiboot header, entry point, and early stack setup.
- `kernel01/kernel/` contains the active kernel implementation, logging, panic handling, and core services.
- `kernel01/kernel/ai/` contains the AI session, serial protocol, HNLang profile, workspace policy, and provider-response handling.
- `kernel01/kernel/graphics/` contains the framebuffer, font, drawing primitives, graphical console, and cursor implementation.
- `kernel01/kernel/interface/` contains the early UI/input event queue and dispatcher.
- `kernel01/kernel/interface/widgets/` contains panel, label, button, and window primitives.
- `kernel01/kernel/interface/screens/` contains launcher applications, including AI Studio.
- `kernel01/kernel/memory/` contains Multiboot memory-map parsing, the physical memory manager, and the early heap.
- `kernel01/kernel/task/` contains the kernel task registry and cooperative scheduler skeleton.
- `kernel01/interrupts/` contains GDT, IDT, exception, and IRQ support.
- `kernel01/fs/` contains the RAM filesystem, VFS skeleton, and secure metadata model.
- `shell/` contains HNShell, its parser, built-in commands, and the task-routing layer.
- `ai/` defines the long-term AI Runtime Interface. AI providers are treated as external services, not trusted kernel components.
- `lang/hnlang/` contains the parser, REPL, and runtime foundations for the HNLang task-oriented system language.
- `ui/` contains HNMos-native UI design areas rather than host application prototypes.
- `sdk/` contains application templates and reference examples for future HNMos applications.
- `tools/ai/` contains the host-side OpenAI bridge.
- `tools/qemu/` contains standard and AI-enabled QEMU launch helpers.
- `tools/test/` contains static checks, policy tests, QEMU smoke tests, and CI helpers.
- `examples/` contains HNLang programs, SDK application concepts, and system behavior examples.
- `tests/` contains kernel, shell, AI, HNLang, and Knowledge FS test targets.
- `.github/workflows/` contains continuous-integration workflows.

## Prerequisites

The default cross-compilation prefix is `i686-elf-`. A complete build generally requires:

- an i686 ELF GCC toolchain;
- GNU binutils for the same target;
- GRUB utilities for ISO generation;
- `xorriso`;
- QEMU with `qemu-system-i386`;
- Python 3 for the optional AI provider bridge.

Run the toolchain check before building:

```sh
make check-tools
./build.sh tools
```

You may override the compiler prefix when using a compatible local toolchain:

```sh
make CROSS_COMPILE=i686-elf-
```

## Build and Run

Build the kernel ELF:

```sh
make kernel
./build.sh kernel
```

Build the bootable ISO:

```sh
make image
./build.sh image
```

Run static verification:

```sh
make verify
./build.sh verify
```

Boot the kernel directly in QEMU:

```sh
make run-kernel
./run-qemu.sh kernel
./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

Boot the ISO in QEMU:

```sh
make run
./run-qemu.sh image
```

Run the AI Studio development environment:

```sh
export OPENAI_API_KEY="your-api-key"
export OPENAI_MODEL="gpt-5.4-mini"
./run-qemu-ai.sh
```

The API key is read by the host-side bridge at runtime. Never place a real key in source files, `.env.example`, shell scripts, screenshots, issue descriptions, or commits. Local `.env` files and common credential formats are ignored by Git.

## AI Studio

AI Studio is available from the kernel launcher as an IDE-style application. It provides a prompt area, an HNLang source editor, provider status, and a controlled apply flow.

The development integration uses this path:

```text
AI Studio in HNMos
        |
        | HNAI1 framed messages over COM2
        v
Host-side Python bridge
        |
        | OpenAI Responses API over HTTPS
        v
Configured OpenAI model
        |
        | generated HNLang candidate
        v
Kernel policy validation
        |
        v
/workspace/main.hn in RAM
```

Security boundaries:

- API credentials are supplied at runtime and are never stored in the repository or kernel image.
- The bridge does not grant the model direct kernel, disk, shell, or host access.
- Provider output is treated as untrusted input.
- Generated content passes size, path, and policy validation before it can be applied.
- The writable target is limited to `/workspace/main.hn`.
- Kernel, boot, driver, policy, audit, and system paths are denied.
- AI operations are intended to accelerate programming and routine tasks, not modify HNMos security boundaries.

The current bridge is intended for development under QEMU. A future production implementation should use a dedicated network stack, encrypted credential storage, explicit capability grants, stronger isolation, and a complete audit interface.

See the following documents for details:

```text
docs/AI_INTEGRATION.md
docs/AI_PROVIDER_BRIDGE.md
docs/AI_PERMISSION_LAYER.md
docs/HNLANG_AI_PROFILE.md
docs/HNLANG_AI_STUDIO.md
```

## HNLang and AI Generation

HNLang is the task-oriented language presented to the AI provider as the preferred HNMos programming language. Its AI profile favors compact, deterministic source code and reusable combinators so that many behaviors can be composed without giving the model unrestricted system authority.

The profile defines:

- a constrained grammar and canonical formatting rules;
- task, pipeline, guard, action, and composition patterns;
- reusable combinators for sequencing, branching, mapping, filtering, retrying, and fallback behavior;
- policy-aware generation instructions;
- a workspace-only output contract;
- examples that can be used as few-shot context by provider adapters.

Example sources are available under `examples/hnlang/`.

## Framebuffer Graphics

Kernel 01 requests a 1024x768x32 graphics mode through the Multiboot video flag in `kernel01/boot/boot.s`. The ISO path uses `gfxmode=1024x768x32,auto` and `gfxpayload=keep` in `tools/image/grub.cfg`. When QEMU's direct Multiboot loader does not provide VBE framebuffer information, the kernel attempts a narrow QEMU/Bochs BGA fallback.

The graphics layer currently provides:

- framebuffer address, width, height, pitch, and bits-per-pixel initialization;
- pixel, rectangle, border, and screen-clear primitives;
- RGB color helpers;
- 8x8 bitmap-font text rendering;
- a graphical console with cursor, newline, wrapping, and basic scrolling;
- PS/2 keyboard input and a graphical `hnmos>` terminal prompt;
- PS/2 mouse input, an on-screen cursor, and pointer status;
- a kernel-side UI event queue and input dispatcher;
- top bar, panels, labels, buttons, windows, and launcher tiles;
- launcher entries for Terminal, System, Memory, AI Studio, and Shutdown;
- framebuffer diagnostics written to the COM1 serial log;
- a safe idle fallback when no framebuffer is available.

Additional subsystem documentation:

```text
docs/FRAMEBUFFER_GRAPHICS.md
docs/GRAPHICAL_CONSOLE.md
docs/KEYBOARD_TERMINAL.md
docs/MOUSE_CURSOR.md
docs/UI_EVENTS.md
docs/UI_PRIMITIVES.md
docs/LAUNCHER.md
docs/MEMORY.md
docs/FILESYSTEM.md
docs/TASKS.md
```

## Filesystem and Tasks

The early RAM filesystem exposes system information such as `/system/version`, `/system/info`, and `/readme.txt`. AI Studio additionally uses the writable in-memory file `/workspace/main.hn`. This workspace is deliberately narrow and does not imply general write access to system files.

The kernel also contains a task registry and a cooperative scheduler foundation. Current HNShell commands include `help`, `version`, `clear`, `halt`, `reboot`, `ls`, `cat`, and `tasks`.

## Linker Script

`linker.ld` places the Kernel 01 image at the 1 MiB address. The Multiboot header is kept in the first section, followed by aligned `.text`, `.rodata`, `.data`, and `.bss` sections.

The script creates a freestanding kernel ELF loaded by a bootloader; it does not link a host operating-system application.

## Boot Artifacts

Kernel ELF:

```text
build/hnmos-kernel01.elf
```

Bootable ISO:

```text
build/hnmos-kernel01.iso
```

The current ISO uses GRUB Multiboot as a practical early-stage boot mechanism. GRUB is not necessarily the final HNMos boot architecture.

## Verification

Run the focused AI and shell checks with:

```sh
make check-ai-policy
make check-ai-bridge
make check-shell
```

Run the complete available verification suite with:

```sh
make verify
```

Some checks require the cross-toolchain, GRUB utilities, and QEMU listed under Prerequisites. A live provider request also requires a valid user-supplied API key and network access.

## Wiki

The [HNMos Wiki](https://github.com/ChiefVenzox/hnmos/wiki) is the home for evolving project guides, design notes, and development workflows. Stable architecture and subsystem documentation remains versioned with the source under [`docs/`](docs/).

Wiki contributions should stay consistent with the current repository state. When a change affects a build command, security boundary, protocol, or public interface, update the corresponding version-controlled document as part of the same pull request.

## Security

HNMos is experimental and is not intended for production or security-critical use. API credentials must never be compiled into the kernel, stored in boot artifacts, or committed to the repository.

Please do not disclose suspected vulnerabilities in a public issue. Follow the private reporting process and supported-version guidance in the [Security Policy](SECURITY.md).

## Development Status

The repository currently covers the original Kernel 01 boot, graphics, input, memory, filesystem, task, shell, policy, SDK, test, and demo milestones. AI Studio extends that foundation with a real development-time provider path while preserving a small, explicit kernel trust boundary.

The project remains experimental. Interfaces, disk formats, language syntax, security policy, and boot behavior may change while the architecture matures.

## License and Contributions

Review the repository license before redistributing the project. Contributions should keep credentials out of version control, preserve the freestanding build, document security-sensitive behavior, and include focused checks for policy or protocol changes.

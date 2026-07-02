# HNlang

HNlang is the native scripting, interface description, and future AI-task language for HNMos.

HNlang source files use the `.hn` extension.

HNlang source files are UTF-8. Early identifiers remain ASCII-only for predictable parsing, while string literals and comments may contain UTF-8 text.

This directory is the Stage 0 design package. It does not add a compiler, interpreter, VM, boot hook, host runtime, or external dependency.

## Purpose

HNlang starts small. Its first job is not to be a large general-purpose language.

Initial use cases:

- Shell automation.
- Internal UI screen description.
- System information display.
- Simple deterministic automation.
- Future AI task descriptions that can be reviewed before execution.

HNlang is designed for HNMos as a bare-metal operating system. Linux, macOS, and Windows are development environments only; HNlang must not depend on them at runtime.

## Design Constraints

- Predictable memory use.
- Static buffers before dynamic allocation.
- No raw kernel memory access.
- No arbitrary hardware access.
- No filesystem writes until the filesystem is stable.
- No networking until HNMos has an approved networking stack.
- Sandboxed by default.
- Kernel services exposed through narrow approved namespaces, not raw pointers.

## Language Stages

```text
Stage 0: Design/spec only
Stage 1: Tokenizer + parser
Stage 2: Interpreter for print, variables, simple commands
Stage 3: UI description support
Stage 4: AI task description support
Stage 5: Bytecode VM if needed later
```

Current status: Stage 0.

## Documents

```text
docs/hnlang/README.md    overview and stage plan
docs/hnlang/spec.md      language schema and architecture
docs/hnlang/examples.md  annotated syntax examples
```

## Example Files

```text
examples/hnlang/hello.hn
examples/hnlang/system_panel.hn
examples/hnlang/ai_task_stub.hn
```

These examples are source samples only. They are not wired into boot.

## Proposed Source Layout

Future implementation should live under:

```text
kernel/lang/hnlang/lexer.*
kernel/lang/hnlang/parser.*
kernel/lang/hnlang/ast.*
kernel/lang/hnlang/interpreter.*
kernel/lang/hnlang/builtins.*
kernel/lang/hnlang/errors.*
```

The current repo may also keep early planning notes under `lang/hnlang/`, but the target runtime path for kernel-side implementation is `kernel/lang/hnlang/`.

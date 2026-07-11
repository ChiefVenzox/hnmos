# HNlang Specification

## Identity

```text
name: HNlang
extension: .hn
purpose: HNMos native scripting/interface/AI-task language
first use case: shell automation and UI screen description
current stage: Stage 0 design/spec only
```

HNlang is a small HNMos-native language for describing safe system actions and simple interface surfaces. It is not a Linux, macOS, or Windows application language. Host operating systems are only development environments.

HNlang must remain suitable for a bootable bare-metal HNMos image.

## Non-Goals For Stage 0

Stage 0 does not implement:

- A compiler.
- A bytecode VM.
- A general-purpose runtime.
- Userspace.
- Filesystem writes.
- AI runtime execution.
- Any boot-time HNlang loading.

## Language Stages

### Stage 0: Design/spec only

Define the language identity, syntax, safety model, namespaces, examples, and proposed implementation layout.

### Stage 1: Tokenizer + parser

Add a fixed-buffer tokenizer and parser that can validate `.hn` source and produce a small AST or parse tree.

Stage 1 should be parse-only by default.

### Stage 2: Interpreter for print, variables, simple commands

Add a tiny interpreter for:

- `print`
- `clear`
- `let`
- simple command declarations
- safe namespace reads

Stage 2 should use predictable memory. Prefer static arena or caller-provided buffers before heap allocation.

### Stage 3: UI description support

Parse and interpret UI declarations such as:

- `app`
- `screen`
- `panel`
- `text`
- `button`
- `route`

Stage 3 should render only through approved HNMos UI APIs.

### Stage 4: AI task description support

Parse AI task declarations and convert them into reviewable task plans.

AI tasks are draft/request objects by default. They must not execute privileged actions directly.

### Stage 5: Bytecode VM if needed later

Only add bytecode if the interpreter becomes too complex or repeated script execution needs a compact representation.

A future VM must still preserve sandboxing, deterministic limits, and bounded memory.

## Core Concepts

### value

A runtime value is one of the supported scalar types or a safe reference to a declared object. Stage 2 should start with strings, integers, and bools only.

### string

A UTF-8 string stored in bounded buffers. Stage 1 may treat strings as validated byte slices with a maximum byte length.

Example:

```hn
"Hello HNMos"
"Merhaba HNMos, dünya"
```

### integer

A signed or unsigned fixed-width integer. Initial implementation should choose one kernel-friendly representation, such as signed 32-bit, and document overflow behavior.

Example:

```hn
let ticks = kernel.ticks
```

### bool

A boolean value:

```hn
true
false
```

### command

A named shell automation block.

```hn
cmd hello {
  print "Hello from HNlang"
}
```

Commands should run through HNShell integration when that exists.

### app

A top-level UI description container.

```hn
app "System" {
  title "System"
}
```

An `app` is not a userspace process in early HNMos. It describes an internal HNMos screen or future app descriptor.

### screen

A named UI surface inside an app.

```hn
screen "Overview" {
  panel "Kernel" {
    text kernel.version
  }
}
```

### panel

A grouped UI region. Early HNlang panels should map to existing kernel-side UI widgets.

### button

A user action trigger. Buttons must call approved actions only.

```hn
button "Memory" {
  route "memory"
}
```

### route

A safe navigation request. In early integration, `route` should map to the screen router, not arbitrary function calls.

```hn
route "launcher"
```

### ai task

A reviewable AI task description.

```hn
ai task "summarize_system" {
  input system.info
  mode "local_or_bridge"
  output console
}
```

AI tasks must pass through policy, permission, and audit layers before execution.

## Basic Syntax

### Print

```hn
print "Hello HNMos"
```

### Variables

```hn
let name = "HNMos"
print name
```

### Command Declaration

```hn
cmd hello {
  print "Hello from HNlang"
}
```

### UI App Description

```hn
app "System" {
  title "System"
  panel "Kernel" {
    text kernel.version
    text kernel.ticks
  }
}
```

### AI Task Description

```hn
ai task "summarize_system" {
  input system.info
  mode "local_or_bridge"
  output console
}
```

HNMOS-CODEX-15 adds the first `ai_bridge` skeleton for this future syntax. Stage 0 HNlang files are not executed yet; later parser work should convert `ai task` blocks into bounded AI task requests before they reach the bridge.

AI Studio introduces a provider-independent HNLang knowledge profile with typed primitives and bounded recipes. The profile may generate drafts only under `/workspace/*.hn`; subscription or API credentials never grant kernel, boot, policy, driver, or audit authority. The primitive catalog, composition recipes, approval boundaries, and transport limitations are defined in `docs/HNLANG_AI_STUDIO.md`.

## Lexical Rules

### Source Encoding

HNlang source files are UTF-8.

Rules:

- `.hn` files should be stored as UTF-8 without BOM.
- String literals and comments may contain UTF-8 text.
- Stage 1 tokenizer should validate UTF-8 in string literals and comments.
- Invalid UTF-8 should be rejected with a clear parse error.
- Line and column tracking may count bytes at first; a later diagnostic layer can count Unicode scalar values for nicer errors.
- Identifiers remain ASCII in early stages for predictable parsing and namespace lookup.

This means user-facing text can be localized:

```hn
print "Merhaba dünya"
print "Sistem hazır"
```

Early framebuffer font support may not render every Unicode glyph yet. The language still preserves UTF-8 text so UI/font work can improve rendering later.

### Comments

```hn
# comment text
# Türkçe yorum UTF-8 olarak saklanır
```

Comments run from `#` to end of line.

### Identifiers

```text
[A-Za-z_][A-Za-z0-9_]*
```

Identifiers are case-sensitive. Unicode identifiers are reserved for a later stage; Stage 1 should keep identifiers ASCII-only.

### Qualified Names

Namespaces use dot-separated identifiers:

```hn
kernel.memory.total
graphics.width
```

### Strings

Strings use double quotes:

```hn
"HNMos"
"Görev tamamlandı"
```

String contents are UTF-8. Length limits should be defined in bytes for Stage 1, because the early kernel parser should stay simple and predictable.

Minimum escapes:

```text
\"  quote
\\  backslash
\n  newline
```

Stage 1 may reject unknown escapes.

### Integers

Decimal integers:

```hn
0
42
1024
```

Hex integers may be added later if kernel debugging needs them.

## Grammar Sketch

This grammar is a target for Stage 1, not an implementation commitment.

```text
program       = statement*
statement     = print_stmt
              | let_stmt
              | clear_stmt
              | route_stmt
              | cmd_decl
              | app_decl
              | ai_task_decl

print_stmt    = "print" expression
let_stmt      = "let" identifier "=" expression
clear_stmt    = "clear"
route_stmt    = "route" string

cmd_decl      = "cmd" identifier "{" statement* "}"
app_decl      = "app" string "{" app_item* "}"
app_item      = title_stmt | screen_decl | panel_decl | button_decl
title_stmt    = "title" string
screen_decl   = "screen" string "{" app_item* "}"
panel_decl    = "panel" string "{" app_item* "}"
button_decl   = "button" string "{" route_stmt "}"

ai_task_decl  = "ai" "task" string "{" ai_item* "}"
ai_item       = "input" expression
              | "mode" string
              | "output" expression

expression    = string | integer | bool | qualified_name | identifier
qualified_name = identifier ("." identifier)+
bool          = "true" | "false"
```

## Standard Namespaces

The following names are read-only in early HNlang:

```text
kernel.version
kernel.ticks
kernel.memory.total
kernel.memory.used
graphics.width
graphics.height
input.keyboard.ready
input.mouse.ready
system.info
```

### Namespace Semantics

`kernel.version` returns a stable HNMos version string.

`kernel.ticks` returns timer ticks if a timer exists. Before timer support, it may return `0` or an unavailable error.

`kernel.memory.total` returns total known memory.

`kernel.memory.used` returns used or reserved memory according to the current HNMos memory accounting model.

`graphics.width` and `graphics.height` return framebuffer dimensions when graphics are available.

`input.keyboard.ready` and `input.mouse.ready` return bool readiness flags.

`system.info` returns a safe system summary object or string.

## Safe Built-In Functions

### print

Writes text to the current console or approved UI output.

```hn
print "Hello HNMos"
```

### clear

Clears the current console or approved text surface. It must not reset router state or kernel state.

```hn
clear
```

### route

Requests navigation through the approved screen router.

```hn
route "launcher"
```

### sleep_ticks

Sleeps or yields for a bounded number of timer ticks if timer support exists.

```hn
sleep_ticks 10
```

If no timer exists, Stage 2 should reject it with a clear error rather than busy-wait forever.

### sysinfo

Prints or returns safe system information.

```hn
sysinfo
```

### meminfo

Prints or returns safe memory information.

```hn
meminfo
```

## Forbidden Operations

HNlang must not expose these operations in early stages:

- Raw pointer access.
- Arbitrary memory reads or writes.
- Hardware port access.
- Direct interrupt modification.
- Direct framebuffer writes outside approved UI API.
- Filesystem writes until FS is stable.
- Network access until networking exists.
- Page table modification.
- Scheduler control.
- Kernel panic or reboot except through explicitly approved system commands.

## Safety Model

HNlang is sandboxed by design.

Rules:

- Source code cannot name kernel symbols.
- Source code cannot store raw addresses.
- Namespaces are read-only unless explicitly documented otherwise.
- Built-ins are whitelisted.
- UI output goes through approved UI primitives.
- AI tasks produce reviewable plans before execution.
- Script execution should have step, recursion, and output limits.
- Stage 1 parser should accept bounded input length.
- Stage 2 interpreter should use bounded command depth.

## Error Model

Errors should be deterministic and user-readable:

```text
hnlang: unexpected token
hnlang: unterminated string
hnlang: invalid utf-8
hnlang: unknown namespace
hnlang: forbidden operation
hnlang: timer unavailable
hnlang: output limit exceeded
```

Stage 1 can report line and column if the tokenizer tracks them.

## Proposed Source Structure

Target implementation layout:

```text
kernel/lang/hnlang/lexer.*
kernel/lang/hnlang/parser.*
kernel/lang/hnlang/ast.*
kernel/lang/hnlang/interpreter.*
kernel/lang/hnlang/builtins.*
kernel/lang/hnlang/errors.*
```

Suggested responsibilities:

```text
lexer.*        tokenization, string scanning, comments
parser.*       grammar validation and AST construction
ast.*          bounded AST node definitions
interpreter.*  Stage 2 execution engine
builtins.*     print, clear, route, sysinfo, meminfo
errors.*       error codes and formatting helpers
```

Use fixed-size tables and caller-provided buffers first. Only introduce heap allocation after the kernel heap is stable enough for language runtime use.

## Future Integration TODOs

- Shell command execution: map `cmd` blocks to HNShell commands.
- Launcher app screens: map `app`, `screen`, `panel`, `button`, and `route` to the UI screen router.
- AI bridge: convert `ai task` blocks to reviewed AI task proposals.
- Package/app model: allow `.hn` metadata to describe internal HNMos apps once app loading exists.
- Filesystem: read `.hn` files from FS only after read path and validation are stable.
- Audit log: record accepted scripts, rejected operations, and executed built-ins.

# HNlang Examples

These examples describe intended HNlang syntax. They are not executed by HNMos in Stage 0.

## Hello

```hn
print "Hello HNMos"
```

Expected future behavior:

```text
Hello HNMos
```

## Variable

```hn
let name = "HNMos"
print name
```

Expected future behavior:

```text
HNMos
```

## UTF-8 Text

```hn
print "Merhaba HNMos, dünya"
print "Sistem hazır"
```

Expected future behavior:

```text
Merhaba HNMos, dünya
Sistem hazır
```

HNlang source files are UTF-8. Early identifiers stay ASCII-only, but strings and comments can carry UTF-8 text.

## Command Block

```hn
cmd hello {
  print "Hello from HNlang"
}
```

Future integration:

- HNShell can expose `hello` as a safe internal command.
- The command body runs through the HNlang interpreter.
- Output uses the approved console or UI output API.

## System UI Panel

```hn
app "System" {
  title "System"
  panel "Kernel" {
    text kernel.version
    text kernel.ticks
  }
}
```

Future integration:

- `app` maps to a launcher-visible internal screen descriptor.
- `panel` maps to kernel UI panel widgets.
- `text kernel.version` reads a safe read-only namespace value.
- `text kernel.ticks` should return `0` or an unavailable value until timer IRQ exists.

## Screen With Route Button

```hn
app "Memory" {
  title "Memory"
  screen "Overview" {
    panel "Memory" {
      text kernel.memory.total
      text kernel.memory.used
      button "Back" {
        route "launcher"
      }
    }
  }
}
```

Future integration:

- `route "launcher"` calls the approved screen router.
- The script cannot call raw kernel functions.

## AI Task Stub

```hn
ai task "summarize_system" {
  input system.info
  mode "local_or_bridge"
  output console
}
```

Future integration:

- Produces a reviewable AI task request.
- Reads only approved `system.info`.
- Writes only to approved `console` output.
- Does not execute shell commands automatically.

## Shell Automation Sketch

```hn
cmd status {
  clear
  print kernel.version
  sysinfo
  meminfo
}
```

Future integration:

- `clear` clears the current text surface only.
- `sysinfo` and `meminfo` read approved kernel summaries.
- No raw memory access is allowed.

## Forbidden Examples

These are intentionally invalid:

```hn
write_ptr 0x100000 "bad"
outb 0x60 0xff
interrupt.disable
framebuffer.write_raw 0 0 0xffffffff
fs.write "/system/config" "bad"
network.open "example.com"
```

Expected future behavior:

```text
hnlang: forbidden operation
```

## Stage 0 Example Files

The repo stores source samples here:

```text
examples/hnlang/hello.hn
examples/hnlang/system_panel.hn
examples/hnlang/ai_task_stub.hn
```

They are examples only and are not wired into boot.

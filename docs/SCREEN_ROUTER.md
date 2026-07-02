# Screen Router

## Hedef

HNMOS-CODEX-08.1 ile grafik launcher ve internal screen gecisleri tek bir kernel-side screen router uzerinden yonetilir.

Bu router bare-metal kernel icindedir. Userspace, process event loop, filesystem app launch veya AI runtime baslatmaz.

## Dosyalar

```text
kernel01/kernel/interface/screen.h
kernel01/kernel/interface/router.*
kernel01/kernel/interface/launcher.*
kernel01/kernel/interface/screens/terminal_screen.*
kernel01/kernel/interface/screens/system_screen.*
kernel01/kernel/interface/screens/memory_screen.*
```

## Screen ID'leri

```text
SCREEN_LAUNCHER
SCREEN_TERMINAL
SCREEN_SYSTEM
SCREEN_MEMORY
SCREEN_AI_PANEL_RESERVED
```

`SCREEN_AI_PANEL_RESERVED` CODEX-15 ile stub AI panelini gosterir. Bu panel gercek AI runtime veya LLM calistirmaz.

## Screen Interface

Her screen sabit bir `struct hnm_screen` kaydi ile router'a baglanir:

```text
screen_init()
screen_render()
screen_handle_event()
screen_on_enter()
screen_on_exit()
```

Router static screen table kullanir. Dynamic allocation yoktur.

## Router API

```text
ui_set_screen(screen_id)
ui_back()
ui_render_current_screen()
ui_dispatch_event_to_current_screen(event)
```

`ui_set_screen()` gecis sirasini su sekilde uygular:

```text
old screen_on_exit
current_screen = next
new screen_on_enter
new screen_render
```

`ui_back()` bu batch'te her zaman Launcher ekranina doner.

## Navigation Keys

Global:

```text
ESC = Terminal/System/Memory ekranlarindan Launcher'a don
F1  = kisa yardim overlay'i
```

Launcher:

```text
1 = Terminal
2 = System
3 = Memory
5 = AI Panel
4 = Shutdown halt
```

Terminal disindaki System/Memory ekranlarinda mevcut kisayol davranisi korunur:

```text
1 = Terminal
2 = System
3 = Memory
5 = AI Panel
4 = Shutdown halt
```

Terminal aktifken `1`-`4` normal terminal karakteridir.

## Terminal Return Commands

Terminalden Launcher'a donmek icin:

```text
back
exit
launcher
```

Bu komutlar prompt basmak yerine `ui_set_screen(SCREEN_LAUNCHER)` cagirir.

Diger terminal komutlari, `halt` ve `reboot` disinda, komut cikisindan sonra tekrar prompt basmalidir.

Ornek:

```text
hnmos> version
HNMos v0.4 keyboard terminal
hnmos>
```

## Clear Davranisi

`clear` grafik terminal console'unu temizler, cursor'u basa alir ve `hnmos>` promptunu yeniden basar.

`clear` router state'ini degistirmez ve aktif screen bilgisini silmez.

## Sinirlar

Bu batch'te yoktur:

- Userspace screen veya app launch.
- Filesystem tabanli program calistirma.
- Desktop/window manager.
- AI runtime.
- Multitasking entegrasyonu.
- Screen history stack.
- Dynamic screen registration.

TODO:

- Back stack gerekiyorsa `ui_back()` icin sabit boyutlu history ekle.
- Timer IRQ geldikten sonra top bar tick alanini gercek sayacla besle.
- Screen bazli help metinlerini daha ayrintili hale getir.

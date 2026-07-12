# HNMos Launcher

## Hedef

HNMOS-CODEX-08 ile Kernel 01 acilista ilk HNMos launcher ekranini gosterir. Bu ekran framebuffer, bitmap font, UI widgets ve UI event queue uzerinde calisan kernel-side bir arayuzdur.

Bu bir desktop, userspace app launcher veya filesystem tabanli program calistirma sistemi degildir. Launcher item'lari yalnizca kernel icindeki internal screen'leri acar.

## Dosyalar

```text
kernel01/kernel/interface/launcher.*
kernel01/kernel/interface/screen.h
kernel01/kernel/interface/router.*
kernel01/kernel/interface/screens/system_screen.*
kernel01/kernel/interface/screens/memory_screen.*
kernel01/kernel/interface/screens/terminal_screen.*
```

Launcher mevcut widget primitive'lerini kullanir:

```text
kernel01/kernel/interface/widgets/button.*
kernel01/kernel/interface/widgets/panel.*
kernel01/kernel/interface/widgets/label.*
kernel01/kernel/interface/widgets/window.*
```

## Boot Davranisi

Framebuffer ve input sistemi hazirlandiktan sonra kernel:

1. UI event queue'yu baslatir.
2. Keyboard ve mouse driver'larini baslatir.
3. Terminal state'ini hazirlar.
4. Screen router'i baslatir.
5. Launcher'i router uzerinden cizer.

Serial log:

```text
launcher: ready.
router: screen router ready.
terminal: hnmos terminal available.
```

Framebuffer yoksa mevcut idle/halt fallback korunur.

## Launcher Ekrani

Launcher ekraninda:

- Ust bar: `HNMos` ve aktif ekran adi.
- Kisayol satiri: `1 terminal 2 system 3 memory 4 shutdown`.
- Merkezi panel.
- Bes item:
  - `1 Terminal`
  - `2 System`
  - `3 Memory`
  - `5 AI Studio` (IDE icon)
  - `4 Shutdown`

Timer IRQ henuz olmadigi icin clock/tick alaninda gercek tick sayaci yoktur. Sistem ekraninda bu acikca `tick: no timer irq yet` olarak gosterilir.

## Screen Router

Aktif ekran bilgisi artik launcher icinde tutulmaz. `kernel01/kernel/interface/router.*` global `current_screen` state'ini tutar ve screen gecislerini yonetir.

Screen ID'leri:

```text
SCREEN_LAUNCHER
SCREEN_TERMINAL
SCREEN_SYSTEM
SCREEN_MEMORY
SCREEN_AI_STUDIO
```

Router API:

```text
ui_set_screen(screen_id)
ui_back()
ui_render_current_screen()
ui_dispatch_event_to_current_screen(event)
```

Gecis sirasinda eski screen `screen_on_exit()`, yeni screen `screen_on_enter()` ve ardindan `screen_render()` cagrilir.

## Kontroller

Keyboard:

```text
1 = Terminal
2 = System
3 = Memory
4 = Shutdown
5 = AI Studio
ESC = Launcher'a don
F1 = Help overlay
```

Mouse:

- Launcher item'lari hover state alir.
- Sol click item'i acar.
- `Shutdown` guvenli halt yoluna girer.

Terminal screen aktifken klavye terminale gider. Bu nedenle `1`-`4` terminal icinde normal karakter olarak davranir. Terminal/System/Memory ekranlarinda `ESC` router uzerinden Launcher'a doner.

## Terminal Screen

Terminal screen, var olan framebuffer graphical console'u yeniden kullanir.

Desteklenen terminal komutlari:

```text
help
version
clear
halt
reboot
ls
cat
tasks
back
exit
launcher
```

Launcher acilis ekraninda terminal otomatik olarak yazmaya baslamaz; Terminal item'i secildiginde console temizlenir ve `hnmos>` promptu gosterilir.

`back`, `exit` ve `launcher` komutlari `ui_set_screen(SCREEN_LAUNCHER)` ile Launcher'a doner. `clear` sadece terminal console'unu temizler ve promptu yeniden basar; router state'ini silmez.

## System Screen

System screen su bilgileri gosterir:

- Kernel: `HNMos Kernel 01`
- Framebuffer resolution ve bpp.
- Keyboard event queue durumu.
- Mouse PS/2 durumu.
- Read-only RAM filesystem node sayisi.
- Kernel task sayisi, current task ve scheduler yield sayisi.
- Timer/tick durumu.

Timer henuz uygulanmadigi icin tick count yerine `no timer irq yet` yazilir.

## AI Studio

AI Studio, provider baglantisi, guvenli HNLang IDE onizlemesi ve workspace policy durumunu gosterir:

```text
AI CONNECTION
HN AI Studio | main.hn | HNLang AI profile
Generate draft | Review | Build request
```

API-key girisi maskelidir; key ozel COM2 bridge kanalina aktarildiktan sonra HNMos tamponu sifirlanir. `G`, kullanici promptunu gercek provider'a yollar ve cevap `/workspace/main.hn` RAM taslagina yazilir. ChatGPT aboneligi API yetkisi olarak kullanilmaz. AI yalnizca workspace taslagi uretebilir; HNMos kernel, boot, policy, driver ve audit alanlari reddedilir. Ayrintilar `docs/HNLANG_AI_STUDIO.md` icindedir. `ESC` ile Launcher'a doner.

## Memory Screen

Memory screen HNMOS-CODEX-09 ile PMM ve heap istatistiklerini gosterir:

- Total memory.
- Usable memory.
- Reserved memory.
- Multiboot mmap entry sayisi.
- PMM free page sayisi.
- Heap used byte sayisi.

Ekran henuz paging, userspace memory ownership veya free-list heap verisi gostermez; bunlar acikca TODO olarak etiketlenir.

## Sinirlar

Bu batch'te yoktur:

- Userspace app launch.
- Filesystem tabanli app listesi.
- Desktop/window manager.
- Real clock veya timer tick counter.
- Back button widget'i veya screen history stack.
- Full memory manager ownership verisi.

TODO:

- PIT/timer geldikten sonra tick indicator ekle.
- Back/Home item'i ekle.
- Launcher item'larini ileride internal kernel services veya userspace app descriptors ile esle.
- Shutdown icin onay ekranini sonraki UI batch'lerinden birine birak.

# hnmos

HNMos, AI destekli, gorev odakli ve sifirdan gelistirilen deneysel bir bare-metal isletim sistemi projesidir.

Bu repo Windows/Linux/macOS uzerinde calisan bir prototip uygulama icin kurulmaz. Gelistirme bilgisayari yalnizca kod yazmak, compiler/assembler/linker calistirmak, boot edilebilir imaj uretmek ve QEMU testleri yapmak icin kullanilir.

Uretilen cikti bir masaustu uygulamasi degil, QEMU'da ve ileride gercek donanimda boot edilebilecek HNMos kernel/imaj ciktisidir.

## Hedef

HNMOS-CODEX-14'un hedefi, HNMos v0.0.1 ilk demo surumunu hazirlamaktir. Demo Windows/Linux/macOS uzerinde calisan uygulama koleksiyonu degil, QEMU uzerinde boot eden gercek bare-metal Kernel 01 baslangicidir.

## Dosya Yapisi

```text
hnmos/
|-- README.md
|-- RELEASE_NOTES.md
|-- Makefile
|-- build.sh
|-- run-qemu.sh
|-- linker.ld
|-- docs/
|-- kernel01/
|   |-- boot/
|   |-- kernel/
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
|   |-- toolchain/
|   |-- qemu/
|   |-- image/
|   `-- test/
|-- examples/
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

## Klasorlerin Amaci

- `docs/`: mimari, build, boot image ve katkida bulunma belgeleri.
- `RELEASE_NOTES.md`: v0.0.1 ve sonraki surum notlari.
- `kernel01/`: ilk boot eden bare-metal HNMos cekirdegi.
- `kernel01/boot/`: Multiboot header, entry point ve erken stack kurulumu.
- `kernel01/kernel/`: `main.c`, `log.*`, `panic.*` ve ilk cekirdek fonksiyonlari.
- `kernel01/kernel/interface/`: erken UI/input event queue ve dispatcher.
- `kernel01/kernel/interface/widgets/`: panel, label, button ve window primitive'leri.
- `kernel01/kernel/interface/screens/`: launcher icin internal kernel screen'leri.
- `kernel01/kernel/memory/`: Multiboot memory map, PMM ve erken kernel heap.
- `kernel01/kernel/task/`: kernel-side task registry ve cooperative scheduler skeleton.
- `kernel01/console/`: erken kernel console, prompt ve console I/O tasarimi.
- `kernel01/memory/`: fiziksel/sanal bellek yonetimi icin ayrilan alan.
- `kernel01/interrupts/`: GDT, IDT, exception ve IRQ kodlari icin ayrilan alan.
- `kernel01/timer/`: PIT, tick sayaci ve timer interrupt altyapisi icin ayrilan alan.
- `kernel01/scheduler/`: kernel task ve round-robin scheduler taslagi icin ayrilan alan.
- `kernel01/syscalls/`: uzun vadeli user-space syscall kapisi icin ayrilan alan.
- `kernel01/events/`: uzun vadeli driver event alanidir; mevcut erken UI/input queue `kernel01/kernel/interface/` altindadir.
- `kernel01/drivers/`: VGA, klavye, disk gibi ilk surucu kodlari icin ayrilan alan.
- `kernel01/fs/`: initramfs, basit FS ve Knowledge FS altyapisi icin ayrilan alan.
- `kernel01/fs/metadata/`: kernel seviyesinde guvenli dosya metadata modeli.
- `shell/`: HNShell kaynaklari. Uzun vadede user-space sistem yuzudur.
- `shell/hnshell/`: HNShell oturum ve runtime cekirdegi.
- `shell/parser/`: komut satiri parser ve token modeli.
- `shell/commands/`: built-in `hn ...` komutlari.
- `shell/commands/events/`: `hn event ...` komutlari.
- `shell/task_router/`: komutlari gorev isteklerine ceviren gecis katmani.
- `ai/`: AI Runtime Interface kaynaklari. AI kernel icine gomulmez; OS arayuzu olarak tasarlanir.
- `ai/runtime_interface/`: HNShell/Task Engine ile model runtime arasindaki AI servis kapisi.
- `ai/task_engine/`: kullanici niyetini izlenebilir gorev kayitlarina ceviren katman.
- `ai/context_router/`: AI'ye verilecek izinli sistem baglamini sinirlayan katman.
- `ai/safety_layer/`: izin, risk ve execution politikalarini uygulayan katman.
- `ai/permission_manifest/`: task, HNShell ve HNLang icin izin manifestleri.
- `ai/action_audit/`: kritik eylem, kullanici onayi ve policy karar loglari.
- `ai/policy_engine/`: capability, risk ve approval kararlarini ureten katman.
- `ai/model_runtime_spec/`: gelecekteki local model runtime adapter sozlesmesi.
- `ai/memory_index/`: Knowledge FS arama ve semantic memory index hedefi.
- `ai/semantic_fs/`: AI Runtime Interface icin semantic FS sorgu arayuzu.
- `ai/indexer/`: metadata, ozet ve etiketlerden index uretecek user-space hedefi.
- `ai/event_reader/`: AI icin izinli ve sanitize edilmis event snapshot okuyucu.
- `lang/`: HNLang kaynaklari. Gorev odakli sistem dili burada gelisir.
- `lang/hnlang/`: HNLang ana dil cekirdegi.
- `lang/hnlang/parser/`: lexer, parser, token ve AST/IR hedefleri.
- `lang/hnlang/repl/`: HNShell icinden acilacak minimal REPL hedefi.
- `lang/hnlang/runtime/`: task IR'i Core OS servislerine yonlendirecek runtime hedefi.
- `ui/`: HNMos'un kendi UI deneyleri; host UI prototipi degildir.
- `ui/console/`: erken text-mode UI, HNShell prompt ve status line hedefi.
- `ui/framebuffer/`: pixel tabanli cizim, font render ve framebuffer console hedefi.
- `ui/window_manager/`: uzun vadeli pencere, focus ve compositor modeli.
- `ui/command_palette/`: gorev odakli kullanici niyet girisi.
- `ui/assistant_panel/`: AI Runtime Interface icin guvenli panel yuzeyi.
- `ui/task_monitor/`: Visual Task Monitor icin gorev durumu ve event gorunumu.
- `sdk/`: HNMos uzerinde calisacak uygulamalar icin SDK tasarim alani.
- `sdk/templates/`: app manifest ve uygulama sablonlari.
- `sdk/examples/`: SDK referans ornekleri.
- `tools/`: toolchain, QEMU ve image uretim yardimcilari.
- `tools/test/`: statik dogrulama, QEMU smoke ve CI test yardimcilari.
- `examples/`: HNLang, HNShell, SDK uygulamalari ve sistem davranisi ornekleri.
- `examples/hn-notes/`: not, metadata ve ozetleme odakli app hedefi.
- `examples/hn-monitor/`: sistem status, event snapshot ve task monitor app hedefi.
- `examples/hn-ai-lab/`: AI Runtime Interface icin guvenli deney app hedefi.
- `examples/demo_task.hn`: v0.0.1 demo gorev ornegi.
- `scripts/`: demo ve release yardimci scriptleri.
- `scripts/demo.sh`: v0.0.1 build, verify ve QEMU demo yardimcisi.
- `tests/`: build, imaj, kernel, shell, AI, HNLang ve Knowledge FS testleri.
- `tests/kernel/`: kernel boot, panic ve low-level behavior test hedefleri.
- `tests/shell/`: HNShell parser ve komut test hedefleri.
- `tests/ai/`: AI Runtime Interface ve permission test hedefleri.
- `tests/hnlang/`: HNLang parser ve REPL test hedefleri.
- `tests/knowledge_fs/`: Knowledge FS metadata test hedefleri.
- `.github/workflows/`: CI workflow ornekleri.

## Build Komutlari

Toolchain kontrolu:

```sh
make check-tools
./build.sh tools
```

Kernel ELF uretimi:

```sh
make kernel
./build.sh kernel
```

Boot edilebilir ISO imaji uretimi:

```sh
make image
./build.sh image
```

Statik kernel dogrulamasi:

```sh
make verify
./build.sh verify
```

QEMU ile dogrudan kernel boot testi:

```sh
make run-kernel
./run-qemu.sh kernel
./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

QEMU ile ISO boot testi:

```sh
make run
./run-qemu.sh image
```

## Framebuffer Graphics

HNMOS-CODEX-02 ile Kernel 01 ilk framebuffer grafik katmanina sahiptir. Kernel hala bare-metal Multiboot akisi ile boot eder; host sistem sadece derleme ve QEMU testi icindir.

Grafik modu `kernel01/boot/boot.s` icindeki Multiboot video flag'i ile 1024x768x32 olarak istenir. ISO yolunda `tools/image/grub.cfg`, `gfxmode=1024x768x32,auto` ve `gfxpayload=keep` kullanir. QEMU'nun dogrudan `-kernel` Multiboot yukleyicisi VBE framebuffer bilgisini vermediginde kernel dar kapsamli QEMU/Bochs BGA fallback yolunu dener.

Framebuffer katmani su dosyalardadir:

```text
kernel01/kernel/graphics/
|-- color.*
|-- framebuffer.*
|-- primitive.*
|-- font.*
|-- console_graphics.*
`-- cursor.*
```

Mevcut ozellikler:

- Framebuffer address, width, height, pitch ve bpp ilklendirme.
- `put_pixel`, `clear_screen`, `fill_rect`, `draw_rect` seviyesinde pixel cizimi.
- RGB renk yardimcilari.
- 8x8 bitmap font ile framebuffer uzerine karakter ve string cizimi.
- Cursor, newline, satir sonu ve basit scroll davranisina sahip grafik console.
- Koyu arka plan, HNMos v0.3 boot metni, status satirlari ve renkli dikdortgenlerden olusan grafik boot ekrani.
- PS/2 klavye IRQ1 input ve `hnmos>` grafik terminal promptu.
- PS/2 mouse IRQ12 input, framebuffer cursor cizimi ve mouse koordinat status alani.
- Keyboard ve mouse icin kernel-side UI event queue ve input dispatcher.
- Kernel-side UI primitive demo: top bar, panel, labels, buttons ve window/box.
- HNMos launcher ekrani: Terminal, System, Memory ve Shutdown item'lari.
- Multiboot memory map tabanli PMM, page allocation ve page-backed bump heap.
- Read-only RAM filesystem ve VFS iskeleti: `/system/version`, `/system/info`, `/readme.txt`.
- Kernel-side task registry ve cooperative scheduler skeleton.
- `help`, `version`, `clear`, `halt`, `reboot`, `ls`, `cat`, `tasks` komutlari.
- Framebuffer degerlerinin COM1 serial log'a yazilmasi.
- Framebuffer yoksa mevcut idle/halt fallback akisi.

QEMU'da beklenen sonuc: koyu bir ekran uzerinde `HNMos v0.3`, `Graphical Console Online`, `Kernel: OK`, `Framebuffer: Ready`, `Font: Ready` metinleri ve yesil/sari/cyan/mor renkli dikdortgenler gorunur.

Daha ayrintili notlar icin:

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

## Linker Script Mantigi

`linker.ld`, Kernel 01 imajini 1 MiB adresinden baslatir. Multiboot header ilk bolumde tutulur, ardindan `.text`, `.rodata`, `.data` ve `.bss` bolumleri hizalanir.

Bu dosya host uygulamasi linklemek icin degil, bootloader tarafindan yuklenecek freestanding kernel ELF dosyasini olusturmak icindir.

## Boot Image Mantigi

Ana kernel ciktisi:

```text
build/hnmos-kernel01.elf
```

Boot edilebilir imaj ciktisi:

```text
build/hnmos-kernel01.iso
```

Bu kademede ISO uretimi icin GRUB Multiboot kullanilir. GRUB kalici HNMos boot mimarisinin son hali degil, erken Kernel 01'i QEMU'da boot ettirmek icin kullanilan gecici ve pratik boot aracidir.

## Ilk Commit Plani

1. Repo iskeleti ve CODEX belgeleri.
2. Kernel 01 boot, linker ve VGA cikisi.
3. Toolchain kontrol scriptleri.
4. Build, image ve QEMU calistirma komutlari.
5. HNMOS-CODEX-09 icin Driver Event Layer tasarimi.
6. HNMOS-CODEX-10 icin UI Layer tasarimi.
7. HNMOS-CODEX-11 icin Security ve AI Permission Layer tasarimi.
8. HNMOS-CODEX-12 icin SDK ve App Model tasarimi.
9. HNMOS-CODEX-13 icin Test, Debug ve CI tasarimi.
10. HNMOS-CODEX-14 icin HNMos v0.0.1 demo surum hazirligi.

## HNMOS-CODEX-14 Tamamlanma Kriterleri

- `hnmos` repo yapisi bare-metal OS gelistirme icin duzenlenmistir.
- `RELEASE_NOTES.md`, `docs/DEMO.md` ve `docs/VERSION_0_0_1.md` hazirdir.
- `examples/demo_task.hn` ve `scripts/demo.sh` hazirdir.
- Kernel 01 boot logunda v0.0.1 demo, console, HNShell ve AI Runtime stub mesajlari vardir.
- `./build.sh` Kernel 01 ELF uretir.
- `make verify` Multiboot dogrulamasini gecer.
- QEMU bulunan ortamda `./run-qemu.sh` Kernel 01'i boot eder.
- v0.0.2 hedefleri belgelenmistir.

# Kernel Boot

## Hedef

HNMOS-CODEX-03'un hedefi, QEMU uzerinde boot eden minimal HNMos Kernel 01'i olusturmaktir.

Bu kademe Windows/Linux/macOS uzerinde calisan bir uygulama degildir. Host sistem sadece derleme, boot edilebilir imaj uretimi ve QEMU calistirma icin kullanilir. Uretilen cikti `build/hnmos-kernel01.elf` kernel dosyasi ve araclar tamamsa `build/hnmos-kernel01.iso` boot imajidir.

## Dosya Yapisi

```text
kernel01/
|-- boot/
|   `-- boot.s
`-- kernel/
    |-- log.c
    |-- log.h
    |-- main.c
    |-- panic.c
    |-- panic.h
    `-- types.h

tools/
|-- image/
|   |-- build-iso.sh
|   `-- grub.cfg
`-- qemu/
    |-- run.sh
    `-- run-qemu.sh

linker.ld
```

## Boot Sureci

Kernel 01 boot akisi su sekildedir:

```text
QEMU
  -> Multiboot loader
  -> kernel01/boot/boot.s
  -> _start
  -> kernel stack
  -> hnm_kernel_main
  -> VGA text output / framebuffer graphics
  -> COM1 serial log
  -> hlt idle loop
```

`kernel01/boot/boot.s`, Multiboot header'i tanimlar. Bootloader kernel'i yuklediginde CPU `_start` sembolune gelir. `_start`, kernel stack'i kurar ve Multiboot magic degeri ile bilgi adresini `hnm_kernel_main` fonksiyonuna aktarir.

## Minimal Kernel Kodu

Kernel 01'in ana entry noktasi:

```text
kernel01/kernel/main.c
```

Ilk mesaj:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
```

Bu mesaj hem VGA text buffer'a hem de serial COM1 log kanalina yazilir.

## Framebuffer Graphics

Kernel 01 artik Multiboot header icinde 1024x768x32 grafik modu ister. ISO boot yolunda GRUB `gfxpayload=keep` ile grafik payload'u korumaya calisir.

Kernel C tarafinda `hnm_framebuffer_init(multiboot_info_addr)` once Multiboot framebuffer alanlarini okur:

- framebuffer address
- width
- height
- pitch
- bits per pixel
- RGB maske bilgisi

QEMU'nun dogrudan `-kernel` Multiboot yolu VBE framebuffer bilgisini saglamadiginda kernel QEMU/Bochs BGA aygitini probe eder ve PCI BAR0 uzerinden linear framebuffer adresini bulur. Bu fallback basarisiz olursa grafik cizim atlanir ve mevcut idle/halt akisina devam edilir.

Basarili durumda serial log'da framebuffer degerleri gorulur ve QEMU ekraninda HNMos grafik console boot metni cizilir. Grafik console, VGA text mode'a bagli degildir; 8x8 bitmap glyph'leri framebuffer pixel'lerine yazar. Ayrintilar `docs/FRAMEBUFFER_GRAPHICS.md` ve `docs/GRAPHICAL_CONSOLE.md` icindedir.

## VGA Text Output

`kernel01/kernel/log.c`, `0xB8000` adresindeki VGA text buffer'a dogrudan yazar.

Bu cikis host terminal API'si degildir. Emule edilen x86 makinenin VGA bellegine dogrudan yazilan bare-metal cikistir.

## Serial Log Mantigi

Kernel 01, COM1 portunu `0x3F8` taban adresinden ilklendirir.

Serial log su amaclarla kullanilir:

- QEMU terminalinde kernel mesajlarini gormek.
- VGA cikisi calismasa bile erken boot durumunu izlemek.
- Panic mesajlarini kaydetmek.

QEMU, serial cikisi host terminaline su parametreyle aktarir:

```text
-serial stdio
```

## Panic Sistemi

Panic sistemi su dosyalardadir:

```text
kernel01/kernel/panic.c
kernel01/kernel/panic.h
```

`hnm_panic`, hata nedenini log'a yazar ve sistemi `cli` + `hlt` dongusunde durdurur. Kernel 01 su anda sadece Multiboot magic degeri beklenen `0x2BADB002` degilse panic'e gider.

## Linker Script Aciklamasi

`linker.ld`, kernel imajini 1 MiB adresinden baslatir:

```text
. = 1M;
```

Bolumler su sirayla yerlestirilir:

- `.multiboot`: bootloader'in kernel'i taniyabilmesi icin korunur.
- `.text`: kod bolumu.
- `.rodata`: salt okunur veri.
- `.data`: yazilabilir veri.
- `.bss`: sifirlanmis global alan ve kernel stack.

Bu script host process'i degil, bootloader tarafindan yuklenecek freestanding kernel ELF dosyasini uretir.

## Boot Edilebilir Imaj Uretimi

Kernel ELF uretimi:

```sh
make kernel
```

Statik dogrulama:

```sh
make verify
```

Boot edilebilir ISO uretimi:

```sh
make image
```

ISO uretimi `tools/image/build-iso.sh` ve `tools/image/grub.cfg` uzerinden yapilir. Bu kademede GRUB, kalici HNMos bootloader tasarimi degil, Kernel 01'i Multiboot ile QEMU'da baslatan gecici boot aracidir.

## QEMU ile Boot Etme

Dogrudan kernel ELF ile:

```sh
./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

Kok wrapper ile:

```sh
./run-qemu.sh kernel
```

ISO ile:

```sh
./run-qemu.sh image
```

QEMU serial log terminalde gorunur. VGA text output ise QEMU ekraninda gorunur.

## Ilk Test Senaryosu

1. Toolchain kontrol edilir:

```sh
make check-tools
```

2. Kernel derlenir:

```sh
make kernel
```

3. ELF ve Multiboot dogrulanir:

```sh
make verify
```

4. QEMU varsa kernel boot edilir:

```sh
./run-qemu.sh kernel
```

Beklenen cikti:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
```

## Tamamlanma Kriterleri

HNMOS-CODEX-03 su kosullarda tamamlanmis sayilir:

- Kernel 01 `kernel01/boot/boot.s` uzerinden gercek boot entry point'e sahiptir.
- Kernel ana kodu `kernel01/kernel/main.c` icindedir.
- VGA text output ve serial COM1 log sistemi vardir.
- Panic sistemi vardir ve `hlt` ile durur.
- `linker.ld` kernel imaj yerlesimini tanimlar.
- `tools/qemu/run.sh` QEMU boot komutunu tasir.
- `make kernel` boot edilebilir kernel ELF dosyasini uretir.
- `make verify` ELF ve Multiboot durumunu dogrular.

## HNMOS-CODEX-04 Notu

CODEX-04, Kernel 01'i CPU olaylarini anlayan cekirdege tasimak icin once asagidaki altyapilari tasarlar:

1. Memory map.
2. Paging.
3. Interrupts.
4. Timer.
5. Basic scheduler.
6. Syscall interface.

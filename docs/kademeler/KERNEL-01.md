# Kernel 01

## Hedef

Kernel 01'in hedefi, HNMos'un ilk gercek bare-metal cekirdegini uretmektir.

Bu kademe sonunda HNMos, Linux/macOS/Windows uzerinde calisan bir program olarak degil, QEMU tarafindan boot edilen freestanding bir kernel olarak calisir. Cekirdek kendi entry point'inden baslar, kendi stack'ini kullanir ve VGA text buffer'a boot bilgisini yazar.

## Dosya Yapisi

```text
.
|-- Makefile
|-- build/
|   `-- hnmos-kernel01.elf
|-- linker.ld
|-- kernel01/
|   |-- boot/
|   |   `-- boot.s
|   |-- kernel/
|   |   |-- log.c
|   |   |-- log.h
|   |   |-- main.c
|   |   |-- panic.c
|   |   |-- panic.h
|   |   `-- types.h
|   |-- memory/
|   |-- interrupts/
|   |-- drivers/
|   `-- fs/
`-- tools/
    |-- image/
    |   `-- grub.cfg
    `-- qemu/
        `-- run-qemu.sh
```

## Uygulanabilir Cikti

Kernel 01 su parcalardan olusur:

- `kernel01/boot/boot.s`: Multiboot header, kernel stack ve `_start` entry point.
- `linker.ld`: Cekirdegin 1 MiB adresine yerlesmesini saglayan linker betigi.
- `kernel01/kernel/main.c`: Host OS kutuphanesi kullanmadan ilk kernel entry fonksiyonunu calistirir.
- `kernel01/kernel/log.*`: VGA text output ve serial COM1 log kanalini tasir.
- `kernel01/kernel/panic.*`: Panic mesajini basar ve sistemi `hlt` dongusunde durdurur.
- `Makefile`: Freestanding derleme, ELF uretimi, ISO uretimi ve QEMU calistirma hedefleri.
- `tools/image/grub.cfg`: ISO boot icin minimum GRUB menu tanimi.

ISO hedefi icin `grub-mkrescue`, `xorriso` ve `mtools` icindeki `mformat` gerekir. Bu araclar yoksa Kernel 01'in ana kaniti olan `build/hnmos-kernel01.elf` yine de `make kernel` ile uretilebilir.

## Sifirdan OS Gelistirme Mantigi

Bu kademe bilincli olarak az sey yapar. Amac, cok ozellik eklemek degil, gercek boot zincirini kanitlamaktir.

Kernel 01 su anda:

- Bellek yonetimi yapmaz.
- Kesme tablosu kurmaz.
- Dosya sistemi icermez.
- HNShell veya AI runtime baslatmaz.
- Host OS uzerinden terminal cikisi kullanmaz.

Buna karsilik su temeli atar:

- CPU, bootloader tarafindan cekirdegin entry point'ine tasinir.
- Stack ve C fonksiyon cagrisi HNMos tarafinda baslatilir.
- Ekran cikisi dogrudan VGA bellegine yazilir.
- Kernel sonsuz `hlt` dongusunde kontrolu elinde tutar.

## Tamamlanma Kriteri

Kernel 01 tamamlanmis sayilmak icin:

- `make kernel` basarili olmalidir.
- `build/hnmos-kernel01.elf` uretilmelidir.
- `grub-file --is-x86-multiboot build/hnmos-kernel01.elf` basarili olmalidir.
- `make run-kernel` QEMU penceresinde `HNMos Kernel 01` yazisini gostermelidir.
- Ekranda Multiboot magic degeri `0x2BADB002` olarak gorulmelidir.

## Sonraki Kademe

`Kernel 02`: GDT, IDT, interrupt stub'lari, port I/O yardimcilari ve temel kernel log altyapisi.

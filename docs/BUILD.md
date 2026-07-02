# Build

## Hedef

Bu belge, HNMos'un host prototip uygulama olarak degil, boot edilebilir bare-metal kernel/imaj olarak nasil derlenecegini tanimlar.

## Dosya Yapisi

```text
hnmos/
|-- Makefile
|-- build.sh
|-- run-qemu.sh
|-- linker.ld
|-- kernel01/
|   |-- boot/
|   `-- kernel/
`-- tools/
    |-- toolchain/
    |-- qemu/
    `-- image/
```

## Build Ciktilari

Build ciktilari `build/` altinda uretilir:

```text
build/
|-- obj/
|-- hnmos-kernel01.elf
|-- hnmos-kernel01.iso
`-- iso-root/
```

`build/` kaynak agacinin parcasi degildir ve `.gitignore` ile disarida tutulur.

## Komutlar

Toolchain kontrolu:

```sh
make check-tools
```

Kernel ELF:

```sh
make kernel
```

Boot edilebilir ISO:

```sh
make image
```

Statik dogrulama:

```sh
make verify
```

Temizlik:

```sh
make clean
```

Kisa script kullanimi:

```sh
./build.sh kernel
./build.sh image
./build.sh verify
./build.sh clean
```

## Build Mantigi

Build sistemi su adimlari uygular:

1. `kernel01/boot/boot.s` assemble edilir.
2. `kernel01/kernel/*.c` freestanding C olarak derlenir.
3. `linker.ld` ile nesneler tek kernel ELF dosyasina yerlestirilir.
4. Istenirse `tools/image/build-iso.sh` ile ISO kok dizini hazirlanir.
5. `grub-mkrescue` ile boot edilebilir ISO uretilir.

## Kernel 01 Klasor Yapisi

```text
kernel01/
|-- boot/
|   `-- boot.s
|-- kernel/
|   |-- log.c
|   |-- log.h
|   |-- main.c
|   |-- panic.c
|   |-- panic.h
|   `-- types.h
|-- memory/
|-- interrupts/
|-- timer/
|-- scheduler/
|-- syscalls/
|-- drivers/
`-- fs/
```

`kernel01/boot/boot.s`, CPU'nun kernel C koduna gecmeden once ihtiyac duydugu en erken kodu icerir. `kernel01/kernel/main.c`, Kernel 01'in ilk C entry fonksiyonunu icerir. `log.c` VGA ve serial cikisi, `panic.c` ise cekirdegin durdurma yolunu tasir.

## Linker Script Mantigi

`linker.ld`, kernel imajini 1 MiB adresinden baslatir. Multiboot header korunur ve bootloader tarafindan bulunabilir kalir. Kod, salt okunur veri, veri ve BSS bolumleri sayfa hizasina yakin duzenlenir.

Bu yaklasim, kernel imajinin host process'i gibi degil, bootloader tarafindan yuklenen freestanding binary olarak calismasini saglar.

## Tamamlanma Kriteri

Bu build kademesi su kosullarda tamamlanmistir:

- `make kernel` kernel ELF uretir.
- `make verify` ELF tipini ve Multiboot uyumlulugunu kontrol eder.
- `make image` gerekli ISO araclari varsa boot edilebilir imaj uretir.
- Build komutlari host uygulamasi degil bare-metal hedef uretir.

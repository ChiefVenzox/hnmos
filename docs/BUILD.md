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
make check-kernel-toolchain
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
make check-shell
make check
```

`make check`, `verify + check-shell` calistirir. QEMU boot, ISO uretimi ve manifest kontrolu bu hedefin parcasi degildir.

Temizlik:

```sh
make clean
```

Kisa script kullanimi:

```sh
./build.sh kernel
./build.sh image
./build.sh verify
./build.sh check
./build.sh clean
```

## Build Mantigi

Build sistemi su adimlari uygular:

1. `kernel01/boot/boot.s` assemble edilir.
2. Kernel alt dizinlerindeki C ve assembly kaynaklari freestanding i686 olarak derlenir.
3. `linker.ld` ile nesneler ELF32 x86 kernel dosyasina yerlestirilir.
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

Kernel CPU baseline'i i686'dir. Kernel henuz FPU/SIMD state init ve context save/restore yapmadigi icin build `-mgeneral-regs-only` ile birlikte `MMX`, `SSE`, `SSE2`, `AVX` ve hardware floating-point kod uretimini kapatir. `make verify`, final binary icinde bu register/talimat siniflarini da reddeder.

ELF header'daki `Machine: Intel 80386` etiketi ELF32 x86 ABI adidir; calistirma baseline'i `-march=i686` olarak sabittir.

## Tamamlanma Kriteri

Bu build kademesi su kosullarda tamamlanmistir:

- `make kernel` kernel ELF uretir.
- `make verify` ELF32/little-endian tipini, nonzero entry'yi, Multiboot header/checksum'ini, undefined symbol bulunmadigini, zorunlu assembly sembol/talimatlarini ve no-SIMD politikasini kontrol eder.
- `make image` gerekli ISO araclari varsa boot edilebilir imaj uretir.
- Build komutlari host uygulamasi degil bare-metal hedef uretir.

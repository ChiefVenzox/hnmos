# HNMos Mimari Amaci

## Hedef

HNMos'un ilk mimari hedefi, host isletim sistemi uzerinde calisan bir uygulama degil, QEMU tarafindan yuklenen ve kendi bellek adres alaninda calisan freestanding bir cekirdek uretmektir.

Bu nedenle ilk kod, grafik arayuz, shell veya AI runtime degil; boot edilebilir minimum cekirdek olmalidir.

## Dosya Yapisi

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
|-- drivers/
`-- fs/

linker.ld
```

## Tasarim Kararlari

- Kernel kodu host OS API'si kullanmaz.
- Cekirdek `-ffreestanding` olarak derlenir.
- Ilk mimari hedef `i386`/`i686` uyumlu QEMU boot akisidir.
- VGA text buffer, ilk gozlemlenebilir cikis kanali olarak kullanilir.
- GRUB Multiboot bu kademede sadece cekirdegi yuklemek icin kullanilir.
- HNShell, HNLang ve AI Runtime daha sonra cekirdek servisleri olgunlastikca eklenecektir.

## Uygulanabilir Plan

1. `kernel01/boot/boot.s` icinde Multiboot header ve `_start` entry point yazilir.
2. Cekirdek stack'i ayrilir.
3. C entry point'i `hnm_kernel_main` cagrilir.
4. Linker betigi cekirdegi 1 MiB adresine yerlestirir.
5. VGA text buffer'a boot durumu yazilir.
6. QEMU hedefleri Makefile'a ve `run-qemu.sh` dosyasina eklenir.

## Tamamlanma Kriteri

Bu mimari baslangic, `make kernel` ile `build/hnmos-kernel01.elf` uretilip `make run-kernel` ile QEMU'da `HNMos Kernel 01` mesaji goruldugunde tamamlanir.

## Sonraki Kademe

`Kernel 02`, CPU descriptor tablolarini, interrupt iskeletini ve port I/O temelini HNMos tarafinda acikca kurar.

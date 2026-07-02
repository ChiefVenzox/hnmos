# Boot Image

## Hedef

Bu belge, HNMos icin boot edilebilir imaj uretme mantigini tanimlar.

HNMos ciktisi host uzerinde calisan bir program degil, QEMU'da veya ileride gercek donanimda boot edilebilecek kernel/imaj dosyasidir.

## Dosya Yapisi

```text
hnmos/
|-- linker.ld
|-- kernel01/
|   |-- boot/
|   `-- kernel/
|-- tools/
|   `-- image/
|       |-- build-iso.sh
|       `-- grub.cfg
`-- build/
    |-- hnmos-kernel01.elf
    `-- hnmos-kernel01.iso
```

## Boot Dosyalarinin Yeri

Erken boot kaynaklari:

```text
kernel01/boot/boot.s
```

Boot image yardimcilari:

```text
tools/image/grub.cfg
tools/image/build-iso.sh
```

Kernel yerlesim sozlesmesi:

```text
linker.ld
```

## Boot Akisi

Kernel 01 boot akisi:

```text
kernel01/boot/boot.s
  -> Multiboot header
  -> _start
  -> kernel stack
  -> hnm_kernel_main
  -> VGA text buffer output
  -> hlt loop
```

ISO boot akisi:

```text
make image
  -> build/hnmos-kernel01.elf
  -> build/iso-root/boot/hnmos-kernel01.elf
  -> tools/image/grub.cfg
  -> grub-mkrescue
  -> build/hnmos-kernel01.iso
```

## GRUB'un Rolu

Bu kademede GRUB, HNMos'un kalici bootloader tasarimi degildir. GRUB, Multiboot uyumlu Kernel 01'i QEMU'da hizli ve dogru bicimde baslatmak icin kullanilan gecici boot aracidir.

Uzun vadede HNMos kendi bootloader/kernel akisina dogru ilerleyecektir.

## QEMU Bare-Metal Boot Testi

Dogrudan kernel ELF testi:

```sh
./run-qemu.sh kernel
```

ISO testi:

```sh
./run-qemu.sh image
```

QEMU testinde HNMos bir host uygulamasi olarak degil, emule edilen x86 makinede boot eden kernel olarak calisir.

## Tamamlanma Kriteri

- Kernel ELF ciktisi tanimlidir.
- ISO image ciktisi tanimlidir.
- Boot dosyalari ve image yardimcilari ayrilmistir.
- QEMU test komutlari bare-metal boot mantigiyla calisir.

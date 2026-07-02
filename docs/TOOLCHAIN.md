# Toolchain

## Hedef

Toolchain, HNMos'un kendisi degildir. Toolchain sadece HNMos kernelini ve boot edilebilir imajini uretmek icin kullanilan host gelistirme katmanidir.

## Host Ortamin Rolu

Windows, Linux veya macOS su isler icin kullanilabilir:

- Kod yazmak.
- Compiler calistirmak.
- Assembler calistirmak.
- Linker calistirmak.
- Boot edilebilir imaj uretmek.
- QEMU calistirmak.

Host ortam su isler icin kullanilmaz:

- HNMos'u masaustu uygulamasi gibi calistirmak.
- HNShell'i host terminal prototipine donusturmek.
- AI Runtime Interface'i host uygulamasi olarak asil sisteme cevirmek.
- Scheduler, driver veya file system davranisini host API'lerine baglamak.

## Gerekli Araclar

Asgari kernel build:

- `make`
- `i686-elf-gcc` veya host `gcc` + `-m32` destegi

QEMU testi:

- `qemu-system-i386`

Boot edilebilir ISO:

- `grub-mkrescue`
- `xorriso`
- `mformat` (`mtools` paketi)

Multiboot dogrulamasi:

- `grub-file`

## Toolchain Kontrolu

```sh
make check-tools
./tools/toolchain/check.sh
```

Script, eksik araclari raporlar. QEMU ve ISO araclari kernel ELF derlemesi icin zorunlu degildir; fakat boot testi ve boot edilebilir imaj icin gerekir.

## Cross Compiler Notu

Tercih edilen yol `i686-elf-gcc` gibi bir cross compiler kullanmaktir. Eger yoksa Makefile host `gcc -m32` yolunu dener.

Uzun vadede `tools/toolchain/` altinda HNMos'a ozel cross compiler kurulum notlari ve otomasyonlari tutulacaktir.

## Tamamlanma Kriteri

- Toolchain ile HNMos OS ayrimi aciktir.
- Host ortam sadece uretim ve test ortami olarak tanimlanmistir.
- Gerekli ve opsiyonel araclar listelenmistir.
- Toolchain kontrol komutu mevcuttur.

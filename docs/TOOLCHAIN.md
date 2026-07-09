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
- `file`, `od`, hedefe uygun `nm` ve `objdump`

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

`check-tools`, secilen compiler ile gercek bir freestanding C compile, x86 assembly ve ELF32/i386 link probe'u yapar. `make check-kernel-toolchain` bunun yalniz kernel build icin zorunlu alt kumesidir. QEMU ve ISO araclari kernel ELF derlemesi icin zorunlu degildir; fakat boot testi ve boot edilebilir imaj icin gerekir.

## Cross Compiler Notu

Tercih edilen yol `i686-elf-gcc` gibi bir cross compiler kullanmaktir. Host `gcc -m32` fallback'i yalniz Linux x86/x86_64 hostlarda denenir. macOS, non-x86 hostlar ve Windows'un POSIX ortamlari `i686-elf-` cross toolchain veya acik `CROSS_COMPILE=/path/to/i686-elf-` ister. Bir executable'in adinin `gcc` olmasi yeterli kabul edilmez; probe yanlis object formatini veya linker'i reddeder.

Repo scriptleri `#!/usr/bin/env sh` ile POSIX shell hedefler. `make check-shell` bunlari hem POSIX `sh` hem `bash --posix` parser'i ile kontrol eder. Bash bir kernel/HNShell runtime bagimliligi degildir ve HNShell `.sh` dosyalarini calistirmaz.

`make verify`, Multiboot header'i ilk 8192 byte icinde ve checksum ile dogrudan kontrol eder. `grub-file` kuruluysa ek bir dogrulama olarak calisir.

Uzun vadede `tools/toolchain/` altinda HNMos'a ozel cross compiler kurulum notlari ve otomasyonlari tutulacaktir.

## Tamamlanma Kriteri

- Toolchain ile HNMos OS ayrimi aciktir.
- Host ortam sadece uretim ve test ortami olarak tanimlanmistir.
- Gerekli ve opsiyonel araclar listelenmistir.
- Toolchain kontrol komutu mevcuttur.

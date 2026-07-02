# Contributing

## Hedef

Bu belge, HNMos'a katkilarin host prototype degil bare-metal OS gelistirme hedefiyle uyumlu kalmasini saglar.

## Temel Kural

HNMos, Windows/Linux/macOS uzerinde calisan bir uygulama olarak tasarlanmaz. Host sistem sadece toolchain ve QEMU calistirma ortami olarak kullanilir.

Katkilar su soruya cevap vermelidir:

```text
Bu degisiklik HNMos'u boot eden, kendi sistem servislerini kuran bagimsiz OS hedefine yaklastiriyor mu?
```

## Gelistirme Akisi

1. Toolchain durumunu kontrol et.
2. Kernel ELF'i derle.
3. Multiboot ve ELF dogrulamasini calistir.
4. QEMU varsa boot testini yap.
5. Degisikligi ilgili docs dosyasiyla birlikte guncelle.

Komutlar:

```sh
make check-tools
make kernel
make verify
make run-kernel
```

## Kabul Edilen Katki Tipleri

- Kernel 01 boot, bellek, interrupt, driver ve FS temelleri.
- Toolchain, build ve image uretim iyilestirmeleri.
- HNShell, HNLang ve AI Runtime Interface icin OS icinde konumlanmis tasarimlar.
- QEMU boot testleri ve statik dogrulamalar.
- Mimari belgeler ve CODEX kademe planlari.

## Kacinilacak Katki Tipleri

- Host uzerinde calisan HNMos masaustu uygulamasi.
- Host terminalinde calisan asil HNShell prototipi.
- AI'yi kernel icine gommek.
- Host dosya sistemi, host thread veya host process API'lerini HNMos servislerinin kalici temeli yapmak.

## Ilk Commit Plani

Ilk commit su kapsami tasimalidir:

1. Repo klasor yapisi.
2. Kernel 01 kaynaklari.
3. `Makefile`, `build.sh`, `run-qemu.sh`.
4. `linker.ld`.
5. Toolchain, QEMU ve boot image yardimcilari.
6. BUILD, TOOLCHAIN, BOOT_IMAGE, CONTRIBUTING ve REPO_STRUCTURE belgeleri.

## HNMOS-CODEX-05'e Gecis

CODEX-05 icin katkilar Multiboot memory map okuma, bellek araliklarini loglama ve ilk fiziksel sayfa ayirici temeline odaklanmalidir.

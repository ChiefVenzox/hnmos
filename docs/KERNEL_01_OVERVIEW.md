# Kernel 01 Overview

## Hedef

Kernel 01, HNMos'un ilk gercek bare-metal boot kanitidir. Bu kademe, HNMos'un host uzerinde calisan bir prototip degil, QEMU tarafindan yuklenen freestanding bir isletim sistemi cekirdegi olacagini gosterir.

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

build/
`-- hnmos-kernel01.elf

tools/
|-- image/
|   `-- grub.cfg
`-- qemu/
    |-- run.sh
    `-- run-qemu.sh
```

## Kernel 01'in Kapsami

Kernel 01 sunlari yapar:

- Multiboot header ile bootloader tarafindan taninir.
- `_start` entry point'inden baslar.
- Kendi kernel stack'ini kurar.
- `hnm_kernel_main` fonksiyonuna gecer.
- VGA text buffer'a dogrudan yazi yazar.
- Multiboot magic ve bilgi adresini ekrana basar.
- Sonsuz `hlt` dongusunde bekler.

Kernel 01 sunlari yapmaz:

- Bellek yonetimi kurmaz.
- Interrupt veya exception handler kurmaz.
- Scheduler calistirmaz.
- Syscall sunmaz.
- Dosya sistemi baglamaz.
- HNShell baslatmaz.
- AI runtime veya AI model calistirmaz.

## Toolchain ile Kernel Ayrimi

Kernel 01'in derlenmesi host ortamda yapilir; fakat calisan hedef host uygulamasi degildir.

Host toolchain gorevleri:

- `boot.s` dosyasini assemble etmek.
- `main.c`, `log.c` ve `panic.c` dosyalarini freestanding olarak derlemek.
- `linker.ld` ile ELF imajini yerlestirmek.
- QEMU'yu kernel ELF ile baslatmak.

Kernel gorevleri:

- Boot sonrasinda CPU kontrolunu almak.
- Kendi bellek adresinde calismak.
- VGA bellegine yazmak.
- Host OS cagrisina ihtiyac duymadan beklemek.

## Kernel/User-Space Durumu

Kernel 01 asamasinda user-space yoktur. Buna ragmen mimari, user-space'i sonraki kademeler icin hedef olarak ayirir.

Kernel 01'de her sey kernel-space sayilir. CODEX-02 ve sonraki kademelerde:

- Exception ve interrupt altyapisi kurulacak.
- Syscall kapisi tasarlanacak.
- User task kavramina hazir veri yapilari olusacak.
- HNShell uzun vadede user-space tarafina tasinacak.

## HNShell ve HNLang'e Etkisi

Kernel 01, HNShell veya HNLang'i calistirmaz. Bunun yerine onlar icin gerekli en temel kosulu saglar: HNMos'un kendi boot eden cekirdek ortamini.

HNShell ancak su cekirdek servisleri olgunlastiktan sonra anlamli hale gelir:

- Klavye girdisi.
- Ekran cikisi.
- Basit bellek ayirici.
- Komut calistirma modeli.
- Dosya veya initramfs erisimi.

HNLang icin de once:

- Bellek yonetimi.
- Dosya okuma.
- Gorev calistirma.
- Izin modeli.
- Syscall arayuzu gerekir.

## AI'nin Kernel 01 Disinda Kalmasi

Kernel 01'e AI eklenmez. Bu karar bilincli bir mimari sinirdir.

AI icin gereken sey boot eden cekirdegin icine model gommek degil, ileride sistem olaylarini ve izinleri temsil edecek dogru OS arayuzlerini tasarlamaktir.

Kernel 01 bu nedenle sadece su soruya cevap verir:

```text
HNMos kendi cekirdegiyle boot edebiliyor mu?
```

## Dogrulama

Kernel 01 icin temel dogrulama adimlari:

```sh
make kernel
grub-file --is-x86-multiboot build/hnmos-kernel01.elf
make run-kernel
```

Beklenen sonuc:

- `build/hnmos-kernel01.elf` uretilir.
- ELF 32-bit i386 executable olarak gorunur.
- Entry point 1 MiB sonrasi kernel adres alanindadir.
- QEMU ekraninda `HNMos Kernel 01` yazisi gorunur.

## Tamamlanma Kriteri

Kernel 01 belgelenmis sayilmak icin:

- Boot zinciri aciklanmistir.
- Host toolchain ile kernel farki belirtilmistir.
- Kernel 01 kapsam disi olan bilesenler yazilmistir.
- HNShell, HNLang ve AI icin neden henuz erken oldugu belirtilmistir.

## HNMOS-CODEX-05'e Gecis

CODEX-05, CODEX-04'te tasarlanan cekirdek altyapilarindan ilkini uygulamaya gecirmelidir.

Ilk teknik hedefler:

- Multiboot memory map okuma.
- Kullanilabilir bellek araliklarini loglama.
- Kernel imaj araligini rezerve etme.
- Ilk fiziksel sayfa ayirici taslagi.
- Paging'e gecmeden once memory map dogrulamasi.

# HNMos Release Notes

## Unreleased - AI Studio

- Launcher'a IDE ikonlu `5 AI Studio` girisi eklendi.
- Subscription OAuth/device-login ve maskeli, gecici API-key yuzeyi eklendi.
- AI credential'larindan bagimsiz deny-by-default workspace policy eklendi.
- HNLang icin typed primitive ve bounded recipe katalogu eklendi.
- `/workspace/*.hn` disindaki AI yazma hedefleri ve kernel/boot/policy/driver/audit yetkileri reddedildi.
- Network ve workspace build istekleri kullanici onayina baglandi.

Kernel 01 dogrudan TCP/TLS yigini icermese de QEMU gelistirme ortaminda COM2 uzerinden host bridge'e baglanir. Bridge, OpenAI Responses API HTTPS istegini yapar ve sonucu policy kontrollu `/workspace/main.hn` RAM taslagina geri yollar.

## v0.0.1 - Kernel 01 Demo

HNMos v0.0.1, QEMU uzerinde boot eden ilk bare-metal demo surumudur.

Bu surum Windows, Linux veya macOS uzerinde calisan bir uygulama koleksiyonu degildir. Host sistem yalnizca kod yazmak, derlemek ve QEMU calistirmak icin kullanilir. Demo ciktisi QEMU uzerinde boot edilebilen HNMos Kernel 01 ELF imajidir.

## Kapsam

v0.0.1 kapsaminda:

- QEMU'da boot eden Kernel 01.
- Multiboot uyumlu 32-bit i386 kernel ELF.
- VGA text output.
- COM1 serial log.
- Baslangic mesaji.
- Panic/log sistemi.
- Minimal console output.
- Minimal HNShell demo stub.
- AI Runtime Interface icin OS ici stub mesajlari.
- Basit gorev komutu demo ciktisi.
- Build, verify ve QEMU calistirma komutlari.
- Demo dokumantasyonu, manifesto ve roadmap.

## Demo Komutlari

Kernel ELF uret:

```sh
./build.sh
```

QEMU'da boot et:

```sh
./run-qemu.sh
```

Demo yardimci script'i:

```sh
./scripts/demo.sh
```

## Boot Edilebilir Cikti

Ana demo ciktisi:

```text
build/hnmos-kernel01.elf
```

Opsiyonel ISO ciktisi, gerekli GRUB/mtools araclari varsa:

```text
build/hnmos-kernel01.iso
```

## Var Olan Ozellikler

- Bare-metal kernel entry.
- Linker script ile 1 MiB yukleme adresi.
- Multiboot header.
- VGA text log.
- Serial log.
- Kernel panic fonksiyonu.
- Kernel halt dongusu.
- Demo console/HNShell/AI stub banner.
- Statik kernel dogrulamasi.
- QEMU smoke test script taslagi.

## Henuz Olmayan Ozellikler

- Gercek klavye input.
- Interaktif HNShell.
- Memory manager.
- Interrupt/IRQ sistemi.
- Timer ve scheduler.
- Syscall sistemi.
- User-space process modeli.
- Gercek AI model runtime.
- Knowledge FS uygulamasi.
- Window manager veya framebuffer UI.
- Kalici dosya sistemi.

## Known Issues

- `qemu-system-i386` kurulu degilse demo script QEMU adimini atlar.
- `./build.sh image` icin `grub-mkrescue`, `xorriso` ve `mformat` gerekebilir.
- Kernel su anda beklenen sekilde `hlt` dongusunde kalir; bu bir demo idle durumudur.
- HNShell ve AI Runtime henuz interaktif degildir; sadece OS ici stub olarak temsil edilir.
- Panic testleri icin ayri test kernel target'i henuz yoktur.

## Sonraki Surum Hedefleri

v0.0.2 icin hedef:

- Minimal klavye input.
- `hn>` prompt.
- `hn status` komut stub'u.
- `hn ai status` komut stub'u.
- `hn task list` komut stub'u.
- Ilk shell parser veri modeli.
- QEMU smoke test'in CI artifact log'u.

## Tamamlanma Kriterleri

v0.0.1 tamam sayilmasi icin:

- `./build.sh` Kernel 01 ELF dosyasini uretmelidir.
- `make verify` ELF ve Multiboot dogrulamasini gecmelidir.
- `./run-qemu.sh` QEMU bulunan ortamda Kernel 01'i boot etmelidir.
- Boot ekraninda HNMos baslangic mesaji gorulmelidir.
- Boot logunda console, HNShell demo ve AI Runtime stub mesajlari gorulmelidir.
- Demo belgeleri ve release notlari repoda bulunmalidir.

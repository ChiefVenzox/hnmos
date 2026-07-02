# HNMos v0.0.1

## Hedef

HNMos v0.0.1, projenin ilk demo surumudur. Bu surumun amaci, HNMos'un host uygulamasi degil, QEMU uzerinde boot eden gercek bare-metal isletim sistemi baslangici oldugunu gostermektir.

## Surum Kapsami

v0.0.1 kapsami:

- Kernel 01.
- Bare-metal boot.
- VGA text log.
- COM1 serial log.
- Panic sistemi.
- Minimal console output.
- HNShell demo stub.
- AI Runtime Interface demo stub.
- Basit gorev komutu demo satirlari.
- Demo task ornegi.
- Build ve QEMU calistirma dokumantasyonu.

## Manifesto

HNMos v0.0.1 su kararlarin ilk somut ciktisidir:

- HNMos bir host OS uygulamasi olmayacak.
- Ilk hedef gercek bare-metal kernel boot akisi olacak.
- AI kernel icine gomulmeyecek.
- Shell ve task sistemi gorev odakli gelisecek.
- Kullanici son yetkili olacak.
- HNLang, Knowledge FS, Driver Event Layer ve SDK asamali ilerleyecek.

## Demo Roadmap

```text
v0.0.1
  -> booting Kernel 01 demo
  -> log/panic/console output
  -> shell and AI stub banners

v0.0.2
  -> keyboard input
  -> hn> prompt
  -> first shell command stubs

v0.0.3
  -> interrupts and timer
  -> event queue
  -> task monitor stub

v0.0.4
  -> memory map and early allocator
  -> shell parser tests
  -> HNLang parser fixture
```

## Build

```sh
./build.sh
```

Ana cikti:

```text
build/hnmos-kernel01.elf
```

## Run

```sh
./run-qemu.sh
```

## Verify

```sh
make verify
```

Beklenen:

```text
ELF 32-bit LSB executable, Intel 80386
Entry point address: 0x101000
multiboot: ok
```

## Known Issues

- Interaktif shell yok.
- Klavye input yok.
- User-space yok.
- AI Runtime gercek model calistirmiyor.
- Knowledge FS henuz tasarim seviyesinde.
- QEMU ve ISO araclari host ortaminda kurulu olmayabilir.

## v0.0.2 Plani

v0.0.2 icin hedefler:

1. Klavye input icin ilk driver yolu.
2. Console input buffer.
3. `hn>` prompt.
4. `hn status` komutu.
5. `hn ai status` komutu.
6. `hn task list` komutu.
7. QEMU smoke test'in release kontrolune baglanmasi.

## Tamamlanma Kriterleri

v0.0.1 tamamlandiginda:

- Kernel 01 QEMU'da boot eder.
- Boot logunda HNMos baslangic mesaji vardir.
- Panic/log sistemi build icindedir.
- Demo console ve HNShell stub mesajlari vardir.
- AI Runtime Interface stub mesajlari vardir.
- `./build.sh` ve `./run-qemu.sh` demo akisini temsil eder.
- Release notes, demo dokumani, version dokumani, demo task ve demo script repoda vardir.

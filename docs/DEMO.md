# HNMos Demo

## Hedef

HNMos v0.0.1 demosunun hedefi, QEMU uzerinde boot eden gercek bare-metal Kernel 01 baslangicini gostermektir.

Bu demo Windows/Linux/macOS uzerinde calisan bir uygulama koleksiyonu degildir. Host sistem sadece derleme ve QEMU calistirma ortami olarak kullanilir.

## Dosya Yapisi

```text
RELEASE_NOTES.md
docs/
|-- DEMO.md
`-- VERSION_0_0_1.md
examples/
`-- demo_task.hn
scripts/
`-- demo.sh
```

## Bare-Metal Demo Senaryosu

Demo akisi:

1. Kernel 01 build edilir.
2. Multiboot uyumlu ELF dogrulanir.
3. QEMU, `build/hnmos-kernel01.elf` dosyasini boot eder.
4. VGA text ve serial log uzerinden baslangic mesajlari gorulur.
5. Minimal console output hazir mesajini verir.
6. Minimal HNShell demo stub komutlari listelenir.
7. AI Runtime Interface henuz model calistirmadan OS ici stub olarak tanitilir.
8. Kernel idle durumuna gecer.

## Build Komutu

Ana demo build komutu:

```sh
./build.sh
```

Bu komut `make kernel` calistirir ve ana ciktisi su dosyadir:

```text
build/hnmos-kernel01.elf
```

Statik dogrulama:

```sh
./build.sh verify
```

## Calistirma Komutu

Ana demo calistirma komutu:

```sh
./run-qemu.sh
```

Bu komut:

```text
make kernel
qemu-system-i386 -kernel build/hnmos-kernel01.elf
```

akisini kullanir.

## Demo Script

Tek komutla demo hazirligi:

```sh
./scripts/demo.sh
```

Script:

- Kernel build eder.
- Statik dogrulama yapar.
- QEMU varsa demo kernel boot komutunu calistirir.
- QEMU yoksa hangi komutun kullanilacagini soyler ve temiz sekilde cikar.

## Boot Edilebilir Imaj Ciktisi

v0.0.1 ana boot ciktisi:

```text
build/hnmos-kernel01.elf
```

Bu dosya QEMU `-kernel` modu ile boot edilebilir Kernel 01 imajidir.

Opsiyonel ISO:

```sh
./build.sh image
```

ISO ciktisi:

```text
build/hnmos-kernel01.iso
```

ISO uretimi icin ek host araclari gerekir.

## Beklenen Boot Log

QEMU ekraninda veya serial log'da beklenen metin:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
HNMos v0.0.1 demo boot.
console: minimal output console ready.
hnshell: offline demo stub ready.
ai-runtime: OS interface stub ready; no model loaded.
task-demo: hn status -> Kernel 01 idle.
task-demo: hn ai status -> AI runtime stub offline.
status: Kernel 01 is idle.
```

## Demo Komutlari

Bu surumde komutlar interaktif degildir; boot log uzerinde demo stub olarak gosterilir.

Hedeflenen komut ailesi:

```text
hn status
hn ai status
hn task list
hn demo task
```

v0.0.2 ile `hn>` prompt ve ilk input buffer hedeflenir.

## Hangi Ozellikler Var?

- QEMU boot eden Kernel 01.
- Baslangic mesaji.
- Panic/log sistemi.
- Minimal console output.
- Minimal HNShell demo stub.
- AI Runtime Interface icin OS ici stub mesajlari.
- Basit task demo satirlari.
- Build, verify ve QEMU calistirma komutlari.
- Release notes ve demo belgeleri.

## Hangi Ozellikler Henuz Yok?

- Interaktif shell input.
- Klavye driver.
- Interrupt ve timer sistemi.
- Scheduler.
- Syscall arayuzu.
- User-space app calistirma.
- Gercek AI Runtime veya local model.
- Dosya sistemi ve Knowledge FS uygulamasi.
- Framebuffer UI ve window manager.

## Known Issues

- QEMU yoksa `./run-qemu.sh` calisamaz.
- ISO uretimi ek araclara baglidir.
- Kernel idle modunda bekler ve otomatik kapanmaz.
- Demo HNShell komutlari su an sadece boot log stub'idir.
- AI Runtime Interface gercek model calistirmaz.

## Sonraki Surum Hedefleri

v0.0.2:

- Minimal keyboard input.
- `hn>` prompt.
- `hn status` parser stub'u.
- `hn ai status` parser stub'u.
- `hn task list` parser stub'u.
- Shell command dispatch veri modeli.
- QEMU serial smoke test'in CI artifact olarak saklanmasi.

## Tamamlanma Kriterleri

- Demo belgeleri repoda bulunur.
- `examples/demo_task.hn` ornek gorev dosyasi bulunur.
- `scripts/demo.sh` calistirilabilir durumdadir.
- `./build.sh` Kernel 01 ELF uretir.
- `make verify` Multiboot dogrulamasini gecer.
- QEMU bulunan ortamda `./run-qemu.sh` HNMos Kernel 01'i boot eder.

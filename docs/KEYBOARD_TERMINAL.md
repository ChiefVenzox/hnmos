# Keyboard Terminal

## Hedef

HNMOS-CODEX-04 ile Kernel 01, QEMU uzerinde PS/2 klavye girdisini IRQ1 ile alir ve framebuffer grafik console uzerindeki `hnmos>` promptuna baglar.

Bu bir host terminal uygulamasi degildir. Klavye girdisi emule edilen bare-metal makinedeki PS/2 veri portundan okunur, komutlar kernel icindeki basit terminal tarafindan islenir.

## Interrupt Kapsami

Mevcut uygulama minimaldir:

- IDT kurulumu vardir.
- 8259 PIC 0x20/0x28 vektorlerine remap edilir.
- IRQ1 keyboard ve IRQ12 mouse icin unmask edilir; slave PIC cascade hatti olan IRQ2 acik tutulur.
- IRQ1 assembly stub'i C klavye handler'ini cagirir.
- IRQ handler sadece scancode okur, UI event queue'ya `KEY_DOWN` ve `KEY_CHAR` event'leri ekler ve PIC EOI gonderir.
- Terminal, input dispatcher uzerinden UI event'lerini interrupt disinda ana idle dongusunde poll eder.

APIC, timer IRQ, exception panic handler'lari ve preemptive scheduler bu batch'e dahil degildir. Kernel task registry ve cooperative scheduler skeleton HNMOS-CODEX-11 ile eklenmistir.

## Klavye Destegi

Klavye driver'i `kernel01/drivers/keyboard.*` icindedir. Eski keyboard-only queue yerine `kernel01/kernel/interface/events.*` icindeki ortak UI event queue kullanilir.

Desteklenenler:

- Lowercase harfler: `a-z`
- Sayilar: `0-9`
- Space
- Filesystem path karakterleri: `/`, `.`, `-`
- Enter
- Backspace
- ESC
- F1

Desteklenmeyenler:

- Shift ve uppercase giris.
- Modifier tuslari.
- Ok tuslari.
- Klavye layout secimi.
- Key repeat politikasi.
- Unicode.

## Grafik Terminal

Terminal kodu `kernel01/console/terminal.*` icindedir. Prompt:

```text
hnmos>
```

Line editor ozellikleri:

- Sabit boyutlu tek satir input buffer.
- Yazilan karakterleri framebuffer console'a basma.
- Backspace ile onceki karakteri gorsel olarak silme.
- Enter ile satiri komut olarak gonderme.
- Bilinmeyen komutlara guvenli cevap.

## Komutlar

```text
help
version
clear
halt
reboot
ls
cat
tasks
sync
ai help
ai sysinfo
ai status
ai asm
ai sync
back
exit
launcher
```

Davranis:

- `help`: komut listesini yazar.
- `version`: grafik terminal surum bilgisini yazar.
- `clear`: framebuffer console'u temizler ve yeni prompt yazar.
- `halt`: `cli; hlt` tabanli guvenli durdurma yoluna girer.
- `reboot`: bos IDT ile triple fault reset yolu kullanir. QEMU `-no-reboot` ile calistirilirse resetten sonra VM yeniden baslatilmaz.
- `ls`: read-only RAM filesystem altindaki node'lari listeler.
- `cat`: read-only RAM filesystem dosyasi okur.
- `tasks`: kernel-side task registry ve cooperative scheduler durumunu listeler.
- `sync`: CPU/RAM ve CPU-side framebuffer checkpoint alir ve ozet sayaclari yazar.
- `ai help`: deterministic AI stub yardim cevabini yazar.
- `ai sysinfo`: deterministic AI stub sistem ozet cevabini yazar.
- `ai status`: aktif AI stub provider durumunu yazar.
- `ai asm`: AI assembly planini ve sanitize edilmis makine snapshot ozetini yazar.
- `ai sync`: kernel-owned CPU/RAM/framebuffer checkpoint'ini AI assembly planina baglar.
- `back`, `exit`, `launcher`: screen router uzerinden Launcher'a doner.

## QEMU Test Notu

QEMU monitor uzerinden otomatik klavye testi icin `sendkey` kullanilabilir:

```text
sendkey h
sendkey e
sendkey l
sendkey p
sendkey ret
```

Serial log, komut debug izlerini ayrica yazar:

```text
ui events: fixed queue ready.
input dispatcher: event polling ready.
keyboard: PS/2 scancode set 1 ready.
interrupts: IDT ready, PIC IRQ1/IRQ12 unmasked.
terminal: hnmos terminal available.
launcher: terminal screen opened.
terminal command: help
```

## Sinirlar

Bu batch'te yoktur:

- Userspace shell.
- Multitasking.
- Full GUI/window manager.
- Persistent command history.
- Real keyboard layout handling.

TODO:

- Exception stubs ve panic entegrasyonu.
- Timer IRQ ve daha genel IRQ dispatch sistemi.
- Shift/uppercase ve sembol tuslari.
- Cursor hareketi ve command history.
- Terminal scroll bolgesi.

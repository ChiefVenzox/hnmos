# Mouse Cursor

## Hedef

HNMOS-CODEX-05 ile Kernel 01, QEMU uzerinde PS/2 mouse girdisini IRQ12 ile alir ve framebuffer uzerinde basit bir cursor cizer.

Bu bir host OS mouse entegrasyonu veya tam GUI degildir. Mouse paketleri emule edilen bare-metal makinedeki PS/2 controller uzerinden okunur; cursor kernel framebuffer kodu tarafindan dogrudan cizilir.

## Dosyalar

```text
kernel01/drivers/mouse.*
kernel01/kernel/graphics/cursor.*
```

Ilgili interrupt dosyalari:

```text
kernel01/interrupts/interrupts.*
kernel01/interrupts/isr.s
```

## PS/2 Mouse Kurulumu

Mouse driver'i `hnm_mouse_init()` icinde su erken adimlari uygular:

- PS/2 output buffer'i temizlenir.
- Auxiliary mouse port `0xA8` controller komutu ile etkinlestirilir.
- Controller config byte okunur.
- IRQ12 enable edilir.
- Mouse clock disable biti temizlenir.
- Mouse'a `set defaults` ve `enable data reporting` komutlari gonderilir.

Basarili durumda serial log:

```text
mouse: PS/2 IRQ12 ready.
interrupts: IDT ready, PIC IRQ1/IRQ12 unmasked.
```

## Paket Isleme

IRQ12 handler sadece kucuk is yapar:

- PS/2 status portu kontrol edilir.
- Mouse data biti set ise port `0x60` okunur.
- 3 byte'lik PS/2 packet biriktirilir.
- X/Y delta ve left/right/middle button state'i kernel state'ine yazilir.
- `MOUSE_MOVE`, `MOUSE_BUTTON_DOWN` ve `MOUSE_BUTTON_UP` UI event'leri ortak queue'ya eklenir.
- PIC EOI gonderilir.

Framebuffer cizimi interrupt handler icinde yapilmaz. Ana kernel idle dongusu `hnm_mouse_poll()` ile dirty state'i okur ve cursor'u gunceller. UI event queue ise `kernel01/kernel/interface/events.*` icindedir ve dispatcher tarafindan ana dongude poll edilir.

## Cursor Cizimi

Cursor kodu `kernel01/kernel/graphics/cursor.*` icindedir.

Mevcut davranis:

- 12x16 basit arrow cursor.
- Cursor altindaki pixel'ler raw framebuffer byte'lari olarak saklanir.
- Hareketten once eski cursor alani restore edilir.
- Yeni cursor konumunda arka plan tekrar saklanir ve arrow cizilir.
- Ekranin sag ust bolgesinde mouse koordinatlari ve button state'i cizilir.

Status ornegi:

```text
mouse x:512 y:384 l:0 r:0 m:0
```

## QEMU Beklentisi

Dogrudan kernel boot testinde QEMU'nun varsayilan PS/2 mouse aygiti hedeflenir. Ek USB mouse destegi bu batch'e dahil degildir.

Manuel monitor testi icin:

```text
mouse_move 80 40
screendump build/hnmos-mouse.ppm
```

Beklenen sonuc:

- HNMos grafik terminal ekrani gorunur.
- Beyaz/siyah arrow cursor gorunur.
- `mouse_move` komutundan sonra cursor konumu ve `mouse x:` status degeri degisir.
- Son event status alaninda `event mouse move ...` gorunur.
- Klavye terminali `hnmos>` promptunda calismaya devam eder.

## Sinirlar

Bu batch'te yoktur:

- USB mouse destegi.
- Wheel/scroll destegi.
- Mouse acceleration.
- Window/button interaction.
- Full GUI/window manager.
- Genel input event bus.

Gecici redraw siniri:

- Cursor kendi altindaki pixel'leri save/restore eder.
- Status alani her mouse redraw'unda yeniden boyanir.
- Cursor terminalin aktif yazdigi bolgenin ustundeyse ana dongu cursor'u terminal poll oncesi gizleyip sonra tekrar cizer; yine de bu tam bir compositor degildir.

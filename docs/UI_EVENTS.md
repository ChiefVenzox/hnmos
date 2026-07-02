# UI Events

## Hedef

HNMOS-CODEX-06 ile Kernel 01 icinde kucuk bir UI/input event sistemi eklendi. Bu sistem keyboard ve mouse input'unu ortak bir kernel-side queue uzerinden gelecekteki arayuz bilesenlerine tasimak icin ilk temeldir.

Bu bir desktop, userspace event loop veya process sistemi degildir. Tum event uretimi ve tuketimi su an kernel icindedir.

## Dosyalar

```text
kernel01/kernel/interface/events.*
kernel01/kernel/interface/input_dispatcher.*
```

Ilgili ureticiler:

```text
kernel01/drivers/keyboard.*
kernel01/drivers/mouse.*
```

Ilgili tuketiciler:

```text
kernel01/kernel/interface/router.*
kernel01/kernel/interface/launcher.*
kernel01/kernel/interface/screens/*.*
kernel01/console/terminal.*
```

## Event Tipleri

Mevcut event enum'u:

```text
KEY_DOWN
KEY_CHAR
MOUSE_MOVE
MOUSE_BUTTON_DOWN
MOUSE_BUTTON_UP
```

Timer IRQ henuz olmadigi icin `TIMER_TICK` event'i bu batch'te uretilmez.

## Event Verisi

`struct hnm_ui_event` icinde bu alanlar vardir:

- `type`: event tipi.
- `sequence`: queue tarafindan verilen artan sira numarasi.
- `x`, `y`: mouse event'leri icin ekran koordinati.
- `dx`, `dy`: mouse move icin ekran deltasi.
- `key_code`: Enter, Backspace, ESC ve F1 gibi ozel keyboard kodlari.
- `scancode`: PS/2 keyboard scancode.
- `button`: button down/up icin left/right/middle.
- `buttons`: mevcut mouse button bit mask.
- `character`: printable keyboard karakteri.

## Queue Tasarimi

Queue `kernel01/kernel/interface/events.c` icinde sabit boyutlu ring buffer'dir.

Ozellikler:

- Static storage kullanir.
- Dynamic allocation yoktur.
- 128 entry ayrilmistir; ring buffer dolu/bos ayrimi icin bir entry bos kalir.
- Full durumda yeni event drop edilir.
- Drop sayaci `hnm_ui_event_dropped_count()` ile okunabilir.
- Son event `hnm_ui_event_recent()` ile okunabilir.

IRQ handler'lar event push eder. Ana kernel idle dongusu interrupt'lari gecici olarak kapatip input dispatcher'i calistirdigi icin mevcut producer/consumer paylasimi kucuk ve deterministiktir.

## Dispatcher

`hnm_input_dispatcher_dispatch(handler)` queue'daki event'leri sirayla poll eder ve verilen handler'a iletir.

Mevcut baglanti:

```text
keyboard IRQ1 -> keyboard driver -> UI event queue
mouse IRQ12  -> mouse driver    -> UI event queue
main loop    -> input dispatcher -> screen router
                                -> active screen handler
```

Launcher mouse event'lerini isler:

- `MOUSE_MOVE`: button hover state guncellenir.
- `MOUSE_BUTTON_DOWN`: hovered button pressed olur.
- `MOUSE_BUTTON_UP`: pressed state temizlenir.

Launcher keyboard shortcut event'lerini de isler:

- `1`: Terminal screen.
- `2`: System screen.
- `3`: Memory screen.
- `4`: Shutdown halt.
- `5`: AI panel.

Router global navigation event'lerini isler:

- `ESC`: Terminal/System/Memory ekranlarindan Launcher'a don.
- `F1`: Kisa help overlay'i goster.

Terminal sadece Terminal screen aktifken keyboard event'lerini isler:

- `KEY_CHAR`: karakteri grafik console'a yazar.
- `KEY_DOWN` + Enter: satiri submit eder.
- `KEY_DOWN` + Backspace: onceki karakteri siler.

Terminal return komutlari:

```text
back
exit
launcher
```

Bu komutlar `ui_set_screen(SCREEN_LAUNCHER)` ile Launcher'a doner.

Mouse event'leri queue'dan gecer ve dispatcher status alaninda gorunur. Cursor cizimi, var olan mouse state + cursor redraw yolu ile korunur.

## Debug Status

Dispatcher, framebuffer hazirsa ekranin sag ust bolgesinde son dispatch edilen event icin kucuk bir status satiri cizer.

Ornekler:

```text
event key char h
event mouse move 592 424
event button down left
```

Mouse cursor status alani ayri kalir:

```text
mouse x:592 y:424 l:0 r:0 m:0
```

Serial log acilista event sistemini dogrular:

```text
ui events: fixed queue ready.
input dispatcher: event polling ready.
```

## Sinirlar

Bu batch'te yoktur:

- Desktop veya window manager.
- Userspace event delivery.
- Process/thread event queues.
- Timer tick event'i.
- Dynamic allocation.
- Event subscription registry.
- Priority veya filtering sistemi.
- Dynamic screen registration.

TODO:

- Timer IRQ eklendiginde `TIMER_TICK` event'i uret.
- Gelecekte router icin sabit boyutlu screen history stack ekle.
- Event overflow durumunu grafik veya serial debug ile daha gorunur yap.
- Mouse cursor state ile UI event state'ini ileride tek input modelinde birlestir.

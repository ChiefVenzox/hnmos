# UI Primitives

## Hedef

HNMOS-CODEX-07 ile Kernel 01 icine ilk grafik UI primitive katmani eklendi. Bu katman framebuffer, font renderer ve UI event queue uzerinde calisir.

Bu bir desktop, userspace app sistemi veya window manager degildir. Widget'lar su an kernel/interface-layer bilesenleridir ve yalnizca demo yuzeyi cizer.

## Dosyalar

```text
kernel01/kernel/interface/theme.*
kernel01/kernel/interface/ui_demo.*
kernel01/kernel/interface/widgets/rect.*
kernel01/kernel/interface/widgets/panel.*
kernel01/kernel/interface/widgets/label.*
kernel01/kernel/interface/widgets/button.*
kernel01/kernel/interface/widgets/window.*
```

## Theme

`theme.*` erken UI renklerini tek yerde toplar:

- Background.
- Top bar.
- Panel ve alternate panel.
- Border.
- Accent ve alternate accent.
- Text ve muted text.
- Button normal, hover ve pressed renkleri.

Theme sabittir ve dynamic allocation kullanmaz.

## Rect ve Hit Testing

`widgets/rect.*` icinde ortak rectangle modeli vardir:

```text
x
y
width
height
```

`hnm_ui_point_in_rect(x, y, rect)` mouse hit testing icin kullanilir. Button hover ve pressed state bu fonksiyonla hesaplanir.

## Label

Label en basit text primitive'idir:

```text
x
y
text
foreground
background
```

Text, kernel 8x8 bitmap font ile framebuffer'a cizilir.

## Panel

Panel basit bir dolgulu kutudur:

```text
rect
background
border
```

Panel body, status alanlari veya grup kutulari icin kullanilir.

## Button

Button su state'leri destekler:

- Normal.
- Hover.
- Pressed.

Button henuz komut calistirmaz veya app launch etmez. Sol mouse button event'i sadece gorsel pressed state ve demo status label'ini gunceller.

## Window

Window primitive'i gercek window manager degildir. Su an yalnizca dekoratif bir box cizer:

- Border.
- Title bar.
- Title text.
- Body fill.

Surukleme, resize, focus, z-order ve close/minimize controls yoktur.

## Demo UI Ve Launcher

`hnm_ui_demo_init()` grafik boot ekranindan sonra su yuzeyi cizer:

- Ust interface preview bar.
- Sag tarafta `Control Surface` baslikli box/window.
- Bir panel.
- Status label'lari.
- `Start`, `Tasks`, `About` button'lari.
- Button hover/pressed feedback.

HNMOS-CODEX-08 ile aktif boot yolu artik launcher ekranidir. `ui_demo.*` primitive referansi olarak kalir; acilista `launcher.*` cizilir.

Launcher primitive'leri kullanarak su ekranlari acar:

- Terminal.
- System.
- Memory.
- Shutdown.

## Event Baglantisi

Ana kernel dongusu input dispatcher'dan event alir ve iki hedefe iletir:

```text
UI demo handler
terminal handler
```

Mouse event'leri demo button hover/pressed state'ini gunceller. Keyboard event'leri terminal tarafindan islenir.

## QEMU Beklentisi

QEMU ekraninda beklenen goruntu:

- Koyu HNMos grafik arka plani.
- Ustte `HNMos interface preview` bari.
- Sag tarafta `Control Surface` window/box.
- Panel icinde status label'lari.
- Uc button: `Start`, `Tasks`, `About`.
- Mouse button uzerine geldiginde hover rengi.
- Sol click sirasinda pressed rengi ve `button: start` gibi demo status.
- Sol altta `hnmos>` terminal promptu.

## Sinirlar

Bu batch'te yoktur:

- Full desktop.
- Window manager.
- Userspace apps.
- App launcher.
- Drag/resize/focus.
- Layout engine.
- Dirty rectangle compositor.
- Widget tree ownership modeli.

TODO:

- Launcher item routing eklendi; sonraki adim userspace olmayan daha zengin internal screen routing.
- Widget id ve focus state tasarimi yap.
- Redraw stratejisini cursor ve terminal ile daha temiz ayir.
- Window manager baslamadan once clipping ve z-order icin minimal model ekle.

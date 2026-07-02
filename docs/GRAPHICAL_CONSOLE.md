# Graphical Console

## Hedef

HNMOS-CODEX-03 ile Kernel 01 framebuffer uzerine kendi metnini cizebilen ilk grafik console katmanina sahiptir. Bu console VGA text mode'a bagli degildir; text glyph'leri kernel icindeki bitmap fonttan framebuffer pixel'lerine cizilir.

## Font Formati

Font `kernel01/kernel/graphics/font.*` icindedir.

- Hucre boyutu: 8x8 pixel.
- Her glyph 8 adet `u8` satirdan olusur.
- Her satirda en soldaki pixel bit 7, en sagdaki pixel bit 0 ile temsil edilir.
- `1` bit foreground rengi, `0` bit background rengi olarak cizilir.
- Font kernel kaynak koduna gomuludur; dosya sistemi veya harici font yukleme yoktur.

Temel API:

```c
hnm_font_draw_char(x, y, character, fg, bg);
hnm_font_draw_string(x, y, text, fg, bg);
```

## Console Davranisi

Console kodu `kernel01/kernel/graphics/console_graphics.*` icindedir.

Console state:

- `cursor_x`
- `cursor_y`
- `columns`
- `rows`
- foreground color
- background color

Temel API:

```c
hnm_console_clear();
hnm_console_put_char(character);
hnm_console_write(text);
hnm_console_write_line(text);
hnm_console_newline();
```

`columns` ve `rows`, framebuffer boyutunun 8x8 font hucrelerine bolunmesiyle hesaplanir. `console_put_char` normal karakterleri cizer, `\n`, `\r` ve `\t` icin basit davranis uygular. Satir sonuna gelindiginde otomatik olarak yeni satira gecer.

## Scrolling

Basit scrolling vardir. Cursor son satirdayken yeni satir gerekirse framebuffer 8 pixel yukari kopyalanir ve en alt 8 pixel'lik satir background rengine temizlenir.

Bu scrolling su an tam ekran console icindir. Gelecekte panel icine sinirli console alanlari eklenirse scroll bolgesi ayrica tanimlanmalidir.

## Boot Ekrani

Framebuffer hazirsa Kernel 01 grafik console boot ekranini cizer:

```text
HNMos v0.3
Graphical Console Online
Kernel: OK
Framebuffer: Ready
Font: Ready
```

Serial log ayridir ve framebuffer degerlerini COM1 uzerinden yazmaya devam eder. Grafik console serial log'a bagli degildir.

## Keyboard Terminal

HNMOS-CODEX-04 ile grafik console, PS/2 klavye girdisi alan `hnmos>` promptuna baglanmistir. Ayrintilar:

```text
docs/KEYBOARD_TERMINAL.md
```

## Sinirlar

Bu asamada yoktur:

- Filesystem veya harici font.
- Userspace console.
- Window manager veya full GUI.

TODO:

- Daha genis ASCII kapsami.
- 8x16 font secenegi.
- Grafik panic ekraninda console kullanimi.
- Console alanini tam ekran yerine dikdortgen bolgeyle sinirlama.

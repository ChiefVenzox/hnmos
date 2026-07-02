# Framebuffer Graphics

## Hedef

Bu belge HNMOS-CODEX-02 ile eklenen ilk pixel tabanli grafik katmanini tanimlar. Bu katman bir host uygulamasi, pencere yoneticisi veya tam GUI degildir. Kernel 01 boot ettikten sonra bare-metal framebuffer uzerine basit bir acilis ekrani cizer ve sonra mevcut idle/halt akisina devam eder.

## Boot Protokolu

Kernel 01 hala Multiboot v1 ile baslatilir. `kernel01/boot/boot.s` Multiboot header icinde su istekleri yapar:

- Sayfa hizali module yukleme.
- Temel bellek bilgisi.
- 1024x768x32 grafik modu istegi.

ISO boot yolunda `tools/image/grub.cfg` GRUB grafik payload'unu korumak icin `gfxmode=1024x768x32,auto` ve `gfxpayload=keep` ayarlarini kullanir.

QEMU'nun dogrudan `-kernel` Multiboot yukleyicisi bu ortamda VBE framebuffer bilgisini vermedigi icin kernel once Multiboot framebuffer bilgisini dener, sonra sadece QEMU/Bochs VGA aygiti bulunursa dar kapsamli BGA fallback yolunu dener. Ikisi de yoksa framebuffer kullanilmaz ve kernel mevcut idle/halt fallback akisinda kalir.

## Framebuffer Ilklendirme

Framebuffer kodu `kernel01/kernel/graphics/framebuffer.*` icindedir.

Ilklendirme sirasinda su alanlar saklanir:

- Framebuffer adresi.
- Genislik.
- Yukseklik.
- Pitch.
- Bits per pixel.
- RGB maske/konum bilgisi.
- Kaynak: `multiboot` veya `qemu-bga`.

Desteklenen pixel formatlari RGB 16, 24 ve 32 bpp'dir. Desteklenmeyen veya eksik mod bilgisi guvenli sekilde reddedilir.

## Modul Yapisi

```text
kernel01/kernel/graphics/
|-- color.c
|-- color.h
|-- framebuffer.c
|-- framebuffer.h
|-- primitive.c
|-- primitive.h
|-- font.c
|-- font.h
|-- console_graphics.c
`-- console_graphics.h
```

Mevcut cizim ozellikleri:

- `hnm_framebuffer_put_pixel(x, y, color)`
- `hnm_framebuffer_clear(color)`
- `hnm_draw_fill_rect(x, y, width, height, color)`
- `hnm_draw_rect(x, y, width, height, color)`
- Basit RGB renk yardimcilari.
- 8x8 bitmap font ile `draw_char` ve `draw_string`.
- Cursor, satir sonu, newline ve basit scroll davranisina sahip grafik console.
- HNMos grafik console boot ekrani ve renkli tanilama seritleri.

Grafik console ayrintilari:

```text
docs/GRAPHICAL_CONSOLE.md
```

## Serial Debug

COM1 serial log zaten mevcut oldugu icin framebuffer degerleri ayni log kanalina yazilir:

```text
graphics: framebuffer ready.
framebuffer source:  qemu-bga
framebuffer address: 0xFD000000
framebuffer width:   0x00000400
framebuffer height:  0x00000300
framebuffer pitch:   0x00001000
framebuffer bpp:     0x00000020
```

Framebuffer yoksa beklenen mesaj:

```text
graphics: framebuffer unavailable; using idle fallback.
```

## QEMU

Kernel ELF derleme:

```sh
make kernel
```

Multiboot dogrulama:

```sh
make verify
```

QEMU dogrudan kernel boot:

```sh
./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

Headless screenshot testi:

```sh
( sleep 2; printf 'screendump build/hnmos-direct.ppm\nquit\n' ) | qemu-system-i386 \
    -machine pc \
    -m 128M \
    -kernel build/hnmos-kernel01.elf \
    -serial file:build/hnmos-direct-serial.log \
    -monitor stdio \
    -display none \
    -no-reboot \
    -no-shutdown
```

Beklenen QEMU sonucu: 1024x768 ekranda koyu arka plan, `HNMos v0.3`, `Graphical Console Online`, `Kernel: OK`, `Framebuffer: Ready`, `Font: Ready` metinleri ve yesil/sari/cyan/mor renkli dikdortgenler gorunur.

## Sinirlar

Bu asamada eklenmeyenler:

- Filesystem.
- User-space.
- Networking.
- AI runtime.
- Multitasking.
- Tam GUI veya window manager.

Sonraki grafik adimlari icin TODO:

- Multiboot/GRUB ISO yolunu toolchain eksiksiz ortamlarda tekrar dogrulamak.
- Daha genis ASCII kapsami ve 8x16 font secenegi.
- Panic ekrani icin grafik fallback.
- Donanimdan gelen farkli RGB formatlarini daha genis test etmek.

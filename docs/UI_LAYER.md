# UI Layer

## Hedef

HNMOS-CODEX-10'un hedefi, HNMos icin kendi console, framebuffer ve ileride window manager katmanlari uzerinden ilerleyecek arayuz mimarisini tasarlamaktir.

Bu katman Windows, Linux veya macOS uzerinde calisan bir masaustu uygulamasi olmayacaktir. Host OS sadece kod yazma, derleme, imaj uretme ve QEMU calistirma ortami olarak kalir. HNMos UI, boot eden Kernel 01 ve sonraki Core OS katmanlarinin ustunde calisacak kendi sistem yuzudur.

## Dosya Yapisi

```text
ui/
|-- console/
|-- framebuffer/
|-- window_manager/
|-- command_palette/
|-- assistant_panel/
`-- task_monitor/

docs/
|-- UI_LAYER.md
`-- HNMOS_UX.md
```

- `ui/console/`: erken text-mode sistem yuzeyi, prompt, status satiri ve HNShell UI adaptoru.
- `ui/framebuffer/`: pixel tabanli cizim, font render, primitifler ve framebuffer console hedefi.
- `ui/window_manager/`: uzun vadeli pencere, panel, focus ve compositor modeli.
- `ui/command_palette/`: gorev odakli komut girisi, intent parse ve Task Engine gecisi.
- `ui/assistant_panel/`: AI Runtime Interface icin guvenli, izinli ve denetlenebilir panel.
- `ui/task_monitor/`: Visual Task Monitor icin gorev durumu, event ve execution gorunumu.

## UI Katman Sirasi

HNMos UI gelisimi su sirayla ilerler:

1. Console UI
2. Framebuffer UI
3. Window Manager
4. Command Palette
5. AI Assistant Panel
6. Visual Task Monitor

Bu sira, Kernel 01'in mevcut boot durumundan baslayip daha zengin grafik ve gorev deneyimine dogru ilerler. Ilk hedef, gosterisli bir masaustu degil, bare-metal sistemle guvenilir etkilesimdir.

## Console UI Tasarimi

Console UI, HNMos'un ilk gercek kullanici yuzudur.

Baslangic hedefleri:

- VGA text output veya serial log ile acilis mesaji.
- Kernel durum satiri.
- HNShell prompt:

```text
hn>
```

- Minimal komut echo ve hata mesaji.
- `hn status`, `hn event list`, `hn ui demo` gibi built-in komutlar icin sade metin ciktisi.

Console UI, klasik terminal gibi gorunebilir; fakat hedefi sadece komut calistirmak degildir. Kullanici niyetini gorev istegine cevirecek ilk yuzeydir.

Ornek:

```text
hn> Projeyi analiz et.
task.request created: project.analyze
status: waiting_for_permission_layer
```

Bu ornekte gercek AI modeli calismak zorunda degildir. Ilk asamada shell, cumleyi Task Engine'e gonderilecek bir intent taslagina cevirir.

## Framebuffer UI Plani

Framebuffer UI, VGA text modundan pixel tabanli cizime gecis katmanidir.

Ilk hedefler:

- Bootloader veya kernel tarafindan framebuffer bilgisini almak.
- Ekran genisligi, yuksekligi, pitch ve pixel formatini saklamak.
- `put_pixel`, `fill_rect`, `draw_char`, `draw_text` primitiflerini yazmak.
- Text console'u framebuffer uzerinde yeniden cizmek.
- Basit status bar ve panel alanlari olusturmak.

Erken framebuffer duzeni:

```text
+------------------------------------------------+
| HNMos Kernel 01 | tick: 000123 | events: 00004 |
+------------------------------------------------+
| hn> _                                           |
|                                                |
| log: HNMos Kernel 01 initialized.              |
| log: AI-native operating layer will be loaded  |
|      later.                                    |
+------------------------------------------------+
```

Framebuffer UI, pencere yoneticisi degildir. Sadece grafik cizim altyapisini ve console'un pixel tabanli surumunu hazirlar.

## Window Manager Hedefi

Window Manager, HNMos'un uzun vadeli grafik sistem yuzudur.

Ilkeler:

- Kernel icine tam masaustu mantigi gomulmez.
- Kernel sadece framebuffer, input event ve guvenli IPC/syscall yuzeyini saglar.
- Window Manager uzun vadede user-space sistem servisi olarak calisir.
- Pencereler gorev, dosya, shell oturumu, task monitor ve AI panel gibi OS nesnelerini temsil eder.

Ilk pencere tipleri:

- Console Window
- Task Window
- File/Knowledge View
- Assistant Panel
- Event Monitor

Pencere modeli:

```text
window {
  id
  title
  owner_task
  bounds
  flags
  focus_state
  event_subscription
}
```

## Command Palette Mantigi

Command Palette, HNMos'un gorev odakli ana giris yuzeyidir.

Kullanici sadece program adi yazmak zorunda kalmaz; yapmak istedigi isi ifade eder:

- `Projeyi analiz et.`
- `Kernel build hatalarini duzelt.`
- `Bu dosyayi optimize et.`
- `Sistemdeki yavasligi acikla.`
- `Bu klasordeki onemli dosyalari ozetle.`

Command Palette akis modeli:

```text
input text
  -> shell/parser
  -> task_router
  -> task.request
  -> permission preview
  -> execution or AI suggestion
```

Ilk asamada Command Palette gercek natural language sistemi olmak zorunda degildir. Komutlari ve cumleleri tipli task isteklerine ceviren deterministik bir stub ile baslayabilir.

## AI Assistant Panel Mantigi

AI Assistant Panel, AI Runtime Interface icin gorsel ve metinsel etkilesim yuzudur.

Guvenlik ilkeleri:

- AI kernel icine gomulmez.
- AI donanimi dogrudan kontrol etmez.
- AI komut calistirmaz; task onerisi veya plan uretir.
- Kritik islemler kullanici onayi ister.
- Context Router, AI'ye yalnizca izinli sistem baglamini verir.

Panelde gosterilecek alanlar:

- Kullanici istegi.
- AI tarafindan onerilen plan.
- Gerekli izinler.
- Okunacak dosya, event veya sistem bilgisi.
- Calistirilacak komut varsa onay bekleme durumu.
- Sonuc ozeti ve audit kaydi.

## Visual Task Monitor

Visual Task Monitor, HNMos'un gorev odakli felsefesini gorunur hale getirir.

Gosterilecek bilgiler:

- Aktif task listesi.
- Task durumlari: `queued`, `waiting_permission`, `running`, `blocked`, `done`, `failed`.
- Event baglantilari: keyboard, timer, disk, network, fs metadata.
- AI onerileri ve kullanici onaylari.
- Syscall ve driver event ozetleri.

Basit task modeli:

```text
task_view {
  task_id
  title
  state
  source
  required_permissions
  related_events
  last_message
}
```

## Ilk Bare-Metal Demo UI Senaryosu

CODEX-10 icin hedeflenen ilk demo uygulama degil, tasarimdir. Ileride uygulanacak ilk UI demo su sekilde olmalidir:

1. QEMU'da Kernel 01 boot eder.
2. Console UI acilis mesaji basar:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
```

3. Status satiri gosterilir:

```text
HNMos UI: console mode | events: 0 | tasks: 0
```

4. `hn ui demo` komutu girilir.
5. Console, Command Palette stub mesajini basar.
6. Framebuffer hazirsa ayni bilgi grafik panel olarak cizilir; degilse VGA text mode ile devam edilir.
7. Visual Task Monitor stub'u tek task kaydi gosterir:

```text
task#1 ui.demo status=done source=console
```

## Kernel/User-Space Siniri

UI katmani Kernel 01'e asiri yuk bindirmemelidir.

Kernel tarafinda kalacaklar:

- Framebuffer address ve mode bilgisi.
- Erken console output.
- Input event toplama.
- Panic ekrani.
- Guvenli syscall/event snapshot kapilari.

User-space hedefi olacaklar:

- Window Manager.
- Command Palette.
- AI Assistant Panel.
- Visual Task Monitor.
- Zengin dosya ve Knowledge FS gorunumleri.

## Tamamlanma Kriterleri

- Console UI tasarimi tanimlanmistir.
- Framebuffer UI plani yazilmistir.
- Window Manager uzun vadeli hedefi belirlenmistir.
- Command Palette'in Task Engine'e baglanma mantigi aciklanmistir.
- AI Assistant Panel'in AI Runtime Interface ile guvenli etkilesimi tanimlanmistir.
- Visual Task Monitor modeli yazilmistir.
- Ilk bare-metal demo UI senaryosu belirlenmistir.

## UI Uygulama Notu

UI tasarimi ilk uygulanabilir bare-metal yuzeye cevrilirken hedef:

1. `kernel01/console/` icin minimal prompt ve input buffer.
2. `ui/console/` icin console status line modeli.
3. `hn ui demo` komut stub'u.
4. Framebuffer detect edilirse primitif cizim planinin ilk C arayuzu.
5. Visual Task Monitor icin tek task snapshot stub'u.

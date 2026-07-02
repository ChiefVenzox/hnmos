# HNMos UX

## Hedef

HNMos UX hedefi, kullanicinin bilgisayarla sadece pencere acip kapatarak degil, gorev vererek iletisim kurdugu bir OS deneyimi tasarlamaktir.

HNMos, TempleOS'un dogrudanlik ve kisisellik fikrinden felsefi olarak ilham alir; fakat HNMos'un UX hedefi kendi kernel, shell, task, AI interface ve permission modeli uzerinde kurulur.

## Temel Felsefe

HNMos arayuzu su soruya cevap verir:

```text
Kullanici bilgisayara ne yapmak istiyor?
```

Bu nedenle UI, sadece uygulama listesi veya pencere yonetimi degildir. Arayuz, kullanici niyetini gorev kaydina ceviren, sistem durumunu aciklayan ve onay gerektiren adimlari gorunur kilan bir sistem katmanidir.

## Ana UX Ilkeleri

- Gorev once gelir, pencere sonra gelir.
- Komut, dosya, event ve AI baglami ayni task kaydinda izlenebilir olmalidir.
- Kernel ve donanim kontrolu kullaniciya acik, AI'ye sinirli olmalidir.
- UI, sistemin ne bildigini ve ne yapmak istedigini saklamamalidir.
- Kritik islemler onay istemelidir.
- Ilk arayuz sade console olabilir; hedef yine gercek bare-metal UI'dir.

## Kullanici Komutlari

Ornek girisler:

```text
Projeyi analiz et.
Kernel build hatalarini duzelt.
Bu dosyayi optimize et.
Sistemdeki yavasligi acikla.
Bu klasordeki onemli dosyalari ozetle.
```

Bu cumleler dogrudan sistem islemi olarak calismaz. Once task istegine cevrilir:

```text
user input
  -> command palette
  -> parser/task router
  -> task request
  -> permission preview
  -> execution path
```

## Console Mode UX

Console mode, HNMos'un ilk UI deneyimidir.

Beklenen ekran:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.

HNMos UI: console mode | events: 0 | tasks: 0
hn>
```

Console mode ozellikleri:

- HNShell prompt.
- Kernel durum ciktisi.
- Event ve task ozetleri.
- Panic durumunda okunabilir hata ekrani.
- Command Palette'in text-mode stub'u.

## Framebuffer Mode UX

Framebuffer mode, console deneyimini pixel tabanli hale getirir.

Ilk grafik yuzey:

- Ustte sistem status bar.
- Ortada console/task cikti alani.
- Altta komut giris satiri.
- Sagda ileride AI Assistant Panel veya Task Monitor alani.

Bu asamada animasyon, tema veya zengin pencere sistemi hedef degildir. Once okunabilir yazi, kararli cizim ve QEMU'da test edilebilir grafik cikti hedeflenir.

## Window Manager UX

Window Manager, HNMos'un ilerideki grafik sistem deneyimidir.

Pencereler uygulama merkezli degil, gorev ve sistem nesnesi merkezli olmalidir:

- Bir task penceresi.
- Bir shell oturumu.
- Bir dosya/metadata gorunumu.
- Bir AI assistant paneli.
- Bir event monitor.

Focus modeli basit tutulur:

- Tek aktif pencere.
- Klavye oncelikli navigation.
- Mouse input ileride Driver Event Layer uzerinden eklenir.

## Command Palette UX

Command Palette, UI'nin en onemli gorev girisidir.

Erken text-mode bicimi:

```text
hn> /task Kernel build hatalarini duzelt.
```

Framebuffer/window manager sonrasi bicimi:

```text
+--------------------------------------+
| Ne yapmak istiyorsun?                |
| Kernel build hatalarini duzelt.      |
+--------------------------------------+
| plan: build log oku, hata ayikla     |
| permission: read build logs          |
+--------------------------------------+
```

Command Palette, kullaniciya plan ve izin ozeti gostermeden kritik islem baslatmamalidir.

## AI Assistant Panel UX

AI Assistant Panel, bir sohbet kutusu gibi davranabilir; fakat OS icinde daha dar ve guvenli bir gorevi vardir.

Panelin gostermesi gerekenler:

- Kullanici istegi.
- AI'nin anladigi task.
- Kullanilacak baglam.
- Gerekli izinler.
- Uygulanacak adimlar.
- Kullanici onayi bekleyen noktalar.
- Sonuc ve audit ozeti.

AI Assistant Panel asla su izlenimi vermemelidir:

```text
AI sistemi serbestce kontrol ediyor.
```

Dogru model:

```text
AI onerir, OS sinirlar, kullanici onaylar, sistem uygular.
```

## Visual Task Monitor UX

Visual Task Monitor, sistemin gorev hafizasidir.

Gosterilecek durumlar:

```text
queued
waiting_permission
running
blocked
done
failed
```

Ornek:

```text
task#12 Kernel build hatalarini duzelt
state: waiting_permission
needs: read build log, inspect source tree
source: command_palette
ai: plan_ready
```

Bu gorunum, HNMos'un AI destekli fakat denetlenebilir OS olmasi icin merkezidir.

## Guvenlik ve Onay UX'i

Kritik islem ornekleri:

- Dosya silme.
- Disk yazma.
- Surucu ayari degistirme.
- Network paketi gonderme.
- Kernel veya boot dosyasi degistirme.
- AI tarafindan onerilen komutu calistirma.

Bu islemler icin UI bir izin ozeti gostermelidir:

```text
permission required:
- read: /system/build.log
- write: /kernel01/kernel/main.c
- run: make verify

approve? [y/N]
```

## Ilk Bare-Metal Demo UX

Ilk demo hedefi:

```text
boot -> console banner -> hn prompt -> hn ui demo -> task monitor stub
```

Beklenen ciktinin text-mode hali:

```text
HNMos UI: console mode
command palette: ready
assistant panel: offline stub
task monitor: 1 demo task
```

Beklenen framebuffer hali:

```text
top bar: HNMos UI
left panel: Console
right panel: Task Monitor
bottom line: Command Palette stub
```

## Tamamlanma Kriterleri

- HNMos UI felsefesi gorev odakli olarak tanimlanmistir.
- Console, framebuffer ve window manager UX sirasina oturtulmustur.
- Command Palette'in kullanici niyetini task'a cevirecegi belirtilmistir.
- AI Assistant Panel'in onay ve baglam sinirlari tanimlanmistir.
- Visual Task Monitor'un gorev durumlarini nasil gosterecegi yazilmistir.
- Ilk bare-metal demo UX senaryosu belirlenmistir.

## UI Uygulama Notu

Bu UX tasarimi Kernel 01 uzerinde calisan ilk console UI davranisina indirilmelidir. Ilk hedef, QEMU'da boot eden kernelin `hn>` promptu, `hn ui demo` ciktisi ve task monitor stub'u gostermesidir.

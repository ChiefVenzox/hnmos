# Kernel Console

## Hedef

HNMOS-CODEX-05'in kernel tarafi hedefi, Kernel 01 icinde minimal console soyutlamasini tasarlamaktir.

Bu console Windows/Linux/macOS terminali degildir. Host ortam sadece derleme ve QEMU calistirma icin kullanilir. Kernel console, HNMos cekirdegi icinde VGA text output, serial log ve ileride klavye input uzerinden calisan erken sistem yuzudur.

## Dosya Yapisi

```text
kernel01/
|-- console/
|   |-- console.c
|   |-- console.h
|   |-- input.c
|   |-- input.h
|   |-- line_editor.c
|   `-- line_editor.h
`-- kernel/
    |-- log.c
    |-- log.h
    |-- panic.c
    `-- panic.h
```

Bu dosyalar CODEX-05 tasarim hedefidir. Bu kademede klasor hazirlanir ve uygulanacak akış tanimlanir.

## Kernel Console Mantigi

Kernel console, erken OS evresinde sistemle konusmak icin kullanilan en dusuk seviyeli komut yuzudur.

Console su isleri yapmalidir:

- Kernel log cikisini tek yerde toplamak.
- VGA text buffer'a yazi yazmak.
- Serial COM1 log kanalina yazi yazmak.
- Klavye driver'i hazir oldugunda satir input almak.
- HNShell'e komut satiri teslim etmek.
- Panic durumunda minimum cikis vermeye devam etmek.

Console su isleri yapmamalidir:

- Host terminal API'sine baglanmak.
- AI Runtime'i kernel icinde calistirmak.
- Dosya sistemi, scheduler veya user-space yerine gecmek.
- Kalici HNShell is mantigini kernel icine gommek.

## Console Input/Output Akisi

Ilk cikis akisi:

```text
hnm_console_write
  -> hnm_log_write
  -> VGA text buffer
  -> COM1 serial
```

Ilk input akisi:

```text
keyboard IRQ
  -> keyboard driver
  -> console input buffer
  -> line editor
  -> HNShell parser
  -> command dispatcher
```

CODEX-05'te input akisi henuz uygulama zorunlulugu tasimaz; fakat kernel console buna gore tasarlanir.

## Minimal Line Editor

Ilk line editor ozellikleri kucuk tutulmalidir:

- Karakter ekleme.
- Backspace.
- Enter ile satir tamamlama.
- Sabit boyutlu input buffer.
- Prompt yazimi.

Ilk prompt:

```text
hnmos>
```

History, completion ve renkli cikis sonraki kademelere birakilir.

## Kernel Komutlari ve User-Space Komutlari

Kernel console erken evrede sadece kernel icinde calisir. Bu nedenle ilk komutlar kernel komutlaridir.

Kernel komutlari:

- Boot durumu.
- Memory map ozeti.
- Build bilgisi.
- Event/tick sayaci.
- Panic testleri.

User-space komutlari:

- HNShell user process olarak calistiginda syscall uzerinden kernel servislerine erisir.
- Dosya sistemi, task listesi ve event listesi user-space tarafindan sorgulanir.
- Kernel pointer veya ham fiziksel bellek user-space'e acilmaz.

Uzun vadede HNShell kernel console'dan ayrilir ve user-space sistem yuzu olur. Kernel console ise debug ve panic kurtarma yuzu olarak kalir.

## Ilk Minimal Console Surumu

Ilk surum su isleri yapmalidir:

1. `hnm_console_init()`
2. `hnm_console_write(const char *text)`
3. `hnm_console_write_line(const char *text)`
4. `hnm_console_prompt()`
5. Sabit input buffer taslagi.

Bu fonksiyonlar baslangicta mevcut `hnm_log_*` fonksiyonlarini kullanabilir. Daha sonra VGA, serial ve keyboard ayrimi console altinda toplanabilir.

## Mevcut Grafik Terminal Uygulamasi

HNMOS-CODEX-04 ile framebuffer grafik console uzerinde calisan ilk terminal uygulanmistir.

Desteklenen komutlar:

- `help`
- `version`
- `clear`
- `halt`
- `reboot`

Desteklenen input:

- Lowercase harfler.
- Sayilar.
- Space.
- Enter.
- Backspace.

Ayrintilar `docs/KEYBOARD_TERMINAL.md` icindedir.

## HNShell'e Gecis Noktasi

Console, tamamlanan komut satirini HNShell'e su soyutlamayla teslim etmelidir:

```text
hnm_hnshell_submit_line(const char *line)
```

Bu fonksiyon ilk asamada kernel icinden cagrilabilir. User-space olgunlastiginda ayni komut modeli syscall tabanli HNShell runtime'a tasinir.

## AI Runtime Interface'e Gecis Noktasi

Kernel console AI Runtime'i dogrudan cagirmamalidir.

Gelecek baglanti noktasi:

```text
HNShell command
  -> task router
  -> event/task record
  -> permission layer
  -> AI Runtime Interface
```

`hn ai status` gibi komutlar ilk surumde sadece "not loaded" veya "planned" cevabi verebilir. AI modeli kernel icinde calistirilmaz.

## Tamamlanma Kriterleri

- `kernel01/console/` klasoru hazirdir.
- Kernel console'un VGA/serial cikis mantigi tanimlanmistir.
- Klavye input ve line editor icin ilk akis yazilmistir.
- Kernel komutlari ile user-space komutlari arasindaki fark belirtilmistir.
- HNShell'e ve AI Runtime Interface'e gecis noktasi tanimlanmistir.

## HNMOS-CODEX-07 HNLang Baglantisi

CODEX-07, kernel console uzerinden HNShell'e ulasan satirlarin ileride HNLang REPL veya parse-only akisina aktarilabilecegini tasarlar:

1. Console satirini HNShell submit arayuzune teslim eder.
2. HNShell `hn lang repl` veya `hn lang parse` komutlarini ayirir.
3. HNLang parser parse-only sonucunu HNShell'e dondurur.
4. Runtime execute etmez; task planini gorunur kilmakla sinirli kalir.
5. Console yalnizca sonucu yazar; HNLang kernel icinde calismaz.

# HNShell

## Hedef

HNMOS-CODEX-05'in shell tarafi hedefi, HNShell'i HNMos Core OS icinde calisacak gorev odakli sistem shell'i olarak tasarlamaktir.

HNShell Windows/Linux/macOS uzerinde calisan bagimsiz bir prototip olmayacaktir. Ilk asamada minimal kernel console uzerinden calisabilir. Uzun vadede user-space sistem servisi veya user process olarak calisip kernel servislerine syscall ile erisir.

## Dosya Yapisi

```text
shell/
|-- hnshell/
|   |-- hnshell.c
|   |-- hnshell.h
|   `-- session.h
|-- parser/
|   |-- parser.c
|   |-- parser.h
|   `-- tokens.h
|-- commands/
|   |-- builtin.c
|   |-- builtin.h
|   |-- system.c
|   |-- task.c
|   `-- ai.c
`-- task_router/
    |-- task_router.c
    |-- task_router.h
    `-- task_request.h
```

Bu dosyalar CODEX-05 tasarim hedefidir. Bu kademede klasorler hazirlanir ve uygulanacak sorumluluklar tanimlanir.

## HNShell Felsefesi

HNShell klasik terminal gibi komut alir, ancak temel amaci sadece metin komutu calistirmak degildir.

HNShell'in amaci:

- Kullanici niyetini sistem komutuna cevirmek.
- Core OS durumunu okunabilir hale getirmek.
- Kernel console ile user-space shell arasinda gecis yolu saglamak.
- Gorevleri Task Engine'e yonlendirmek.
- Ileride AI Runtime Interface ile izinli ve denetlenebilir baglanti kurmak.

HNShell, TempleOS'un dogrudan bilgisayarla konusma hissinden ilham alabilir; fakat HNMos icinde gorev, izin ve olay modelleriyle ayrisan kendi shell tasarimina sahip olmalidir.

## Console Input/Output Akisi

Ilk asama:

```text
kernel console
  -> line buffer
  -> hnshell submit line
  -> parser
  -> built-in command dispatcher
  -> console output
```

Uzun vadeli asama:

```text
keyboard driver
  -> console/session input
  -> HNShell user-space runtime
  -> parser
  -> task router
  -> syscall / Core OS service
  -> event log
  -> optional AI Runtime Interface
```

## Komut Parser Yapisi

Parser ilk asamada boslukla ayrilmis token modelini kullanabilir.

Ornek:

```text
hn system inspect memory
```

Token'lar:

```text
argv[0] = hn
argv[1] = system
argv[2] = inspect
argv[3] = memory
```

Ilk parser sorumluluklari:

- Satiri token'lara ayirmak.
- Maksimum token sayisini sinirlamak.
- Maksimum satir uzunlugunu sinirlamak.
- Bilinmeyen komut icin hata dondurmek.
- Built-in dispatcher'a temiz argv vermek.

Quote, pipe, redirect, wildcard ve script destegi sonraki kademelere birakilir.

## Built-in Komutlar

Ilk minimal komut seti:

```text
hn status
hn system inspect memory
hn build info
hn task list
hn event list
hn ai status
```

### hn status

Kernel/Core OS durum ozeti verir.

Ilk cevap:

```text
HNMos Kernel 01: running
HNShell: kernel-console mode
```

### hn system inspect memory

Memory map ve fiziksel bellek yonetimi hazir oldugunda bellek ozetini verir.

Ilk cevap:

```text
memory inspection: planned
```

### hn build info

Kernel build ve boot bilgisi verir.

Ilk cevap:

```text
target: i686 bare-metal
image: hnmos-kernel01
```

### hn task list

Scheduler ve task engine hazir oldugunda task listesini verir.

Ilk cevap:

```text
task engine: not loaded
```

### hn event list

Driver event layer ve kernel event log hazir oldugunda olaylari listeler.

Ilk cevap:

```text
event layer: not loaded
```

### hn ai status

AI Runtime Interface durumunu bildirir.

Ilk cevap:

```text
AI Runtime Interface: not loaded
```

Bu komut AI'yi baslatmaz. Sadece durum sorgusu icin ayrilir.

## Task Engine'e Gecis Noktasi

HNShell, komutlari ikiye ayirmalidir:

- Dogrudan built-in sistem komutlari.
- Gorev olarak Task Engine'e yonlendirilecek komutlar.

Task Router ilk olarak su yapida dusunulur:

```text
parsed command
  -> command kind
  -> task request
  -> task engine queue
  -> scheduler / event layer
```

Ilk task request alanlari:

```text
task_id
command_name
argv
source_session
permission_level
state
```

## AI Runtime Interface'e Gecis Noktasi

AI Runtime Interface, HNShell'in icine gomulmez ve kernel icinde calismaz.

Gelecek akış:

```text
hn ai status
  -> hnshell command dispatcher
  -> AI Runtime Interface status service
  -> permission check
  -> audit/event log
  -> shell output
```

AI ile ilgili komutlar ilk asamada sadece durum bildirir. AI'nin sistem eylemi onermesi veya calistirmasi daha sonra permission layer ve audit log ile tasarlanacaktir.

## Kernel Komutlari ve User-Space Komutlari

Kernel console modunda HNShell:

- Kernel icinden cagrilir.
- Sabit built-in cevaplar verebilir.
- Sadece guvenli debug bilgisi gostermelidir.
- Kernel pointer veya ham bellek acmamalidir.

User-space modunda HNShell:

- Ayrilmis task/process olarak calisir.
- Kernel servislerine syscall ile erisir.
- Dosya sistemi, event layer ve task engine ile kontrollu etkilesir.
- AI Runtime Interface'e permission layer uzerinden ulasir.

## Ilk Minimal Shell Surumu

Ilk surumun kapsami:

1. `hn>` prompt.
2. Satir bazli input.
3. Boslukla ayrilan token parser.
4. Built-in komut dispatcher.
5. Yukaridaki alti `hn ...` komutuna sabit veya kernelden gelen cevap.
6. Bilinmeyen komut icin hata mesaji.

Ilk hata mesaji:

```text
hn: unknown command
```

## Tamamlanma Kriterleri

- `shell/hnshell/`, `shell/parser/`, `shell/commands/`, `shell/task_router/` klasorleri hazirdir.
- HNShell'in host prototipi olmayacagi belirtilmistir.
- Console input/output akisi tanimlanmistir.
- Parser ve built-in komut modeli yazilmistir.
- Task Engine ve AI Runtime Interface gecis noktalari tanimlanmistir.
- Kernel komutlari ile user-space komutlari ayrilmistir.

## HNMOS-CODEX-07 HNLang Baglantisi

CODEX-07, HNShell'in HNLang ile gorev script'i seviyesinde baglanmasini tasarlar:

1. `hn lang repl` HNLang REPL hedefi olarak ayrilir.
2. `hn lang parse <inline-script>` parse-only akis icin planlanir.
3. `hn lang run <task-name>` ileride izinli runtime akisina baglanir.
4. AI tarafindan uretilen HNLang kodu HNShell'de gorunur draft olarak sunulur.
5. User-space'e tasinacak sinirlar dokumanda korunur.

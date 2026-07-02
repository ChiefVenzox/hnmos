# HNLang

## HNMOS-CODEX-12 Notu

Guncel Stage 0 HNlang schema ve mimari plani `docs/hnlang/` altindadir:

```text
docs/hnlang/README.md
docs/hnlang/spec.md
docs/hnlang/examples.md
```

Bu eski belge HNLang'in daha genis ve uzun vadeli vizyonunu korur. CODEX-12 icin baglayici ilk adim, boot'a baglanmayan, sandboxed, kernel-side scripting/interface tasarimidir. Uzun vadede user-space runtime veya bytecode VM dusunulebilir; ancak ilk uygulama bare-metal kisitlari, sabit bellek ve onayli kernel API'leri uzerinden ilerlemelidir.

## Hedef

HNMOS-CODEX-07'nin hedefi, HNMos icin HNLang adinda gorev odakli sistem dilini tasarlamaktir.

HNLang, TempleOS'taki HolyC fikrinden felsefi olarak ilham alabilir: sistemle dogrudan konusabilen, sade ve kisisel bir dil. Ancak HNLang, HNMos'a ozel, modern, gorev odakli, HNShell ve AI Runtime Interface ile uyumlu bir sistem dili olacaktir.

## Ana Karar

HNLang Windows/Linux/macOS uzerinde calisan gecici bir script dili olarak tasarlanmaz.

HNLang'in hedefi, HNMos'un kendi sistem dili olmaktir. Ilk asamada minimal yorumlayici veya REPL planlanabilir; fakat bu plan HNMos Core OS icinde, kernel/user-space sinirlarina saygili bir hedefe gore yapilir.

## Dosya Yapisi

```text
lang/
`-- hnlang/
    |-- parser/
    |-- repl/
    `-- runtime/

docs/
|-- HNLANG.md
`-- HNLANG_SYNTAX.md
```

Gelecek hedef dosyalar:

```text
lang/hnlang/
|-- parser/
|   |-- lexer.c
|   |-- lexer.h
|   |-- parser.c
|   |-- parser.h
|   `-- ast.h
|-- repl/
|   |-- repl.c
|   |-- repl.h
|   `-- repl_session.h
`-- runtime/
    |-- runtime.c
    |-- runtime.h
    |-- value.h
    |-- task_runtime.c
    `-- task_runtime.h
```

## HNLang Amaci

HNLang su amaclara hizmet eder:

- HNMos sistem gorevlerini ifade etmek.
- HNShell komutlarini daha yapisal task script'lerine cevirmek.
- AI tarafindan uretilen planlari denetlenebilir dile indirgemek.
- Kernel servislerine dogrudan degil, syscall ve Core OS servisleri uzerinden erismek.
- Task Engine, Context Router ve Safety Layer ile uyumlu calismak.

HNLang bir genel amacli host scripting dili degil, HNMos icin sistem gorev dili olmalidir.

## Mimari Konum

```text
Kernel 01
  -> Syscall / Event Interface
  -> HNShell
  -> HNLang parser / REPL
  -> HNLang runtime
  -> Task Engine
  -> AI Runtime Interface
```

HNLang kernel icinde calismaz. Uzun vadede user-space runtime olarak calisir. Erken asamada kernel console uzerinden test edilen minimal REPL olabilir; bu REPL kalici host prototip anlamina gelmez.

## Parser Plani

Parser iki asamada gelismelidir:

1. Lexer: token uretir.
2. Parser: AST veya task IR uretir.

Ilk token turleri:

- Identifier.
- String literal.
- Number.
- Keyword.
- `{`
- `}`
- `(`
- `)`
- `.`
- `,`

Ilk keyword'ler:

- `task`
- `read`
- `summarize`
- `output`
- `kernel`
- `ai`
- `shell`

Ilk parser hedefi tam programlama dili degil, task bloklarini ve sistem cagrisi ifadelerini anlayan kucuk bir dil cekirdegi olmalidir.

## REPL Plani

HNLang REPL, HNShell icinden acilabilen minimal bir dil oturumu olarak tasarlanir.

Ilk REPL prompt:

```text
hnlang>
```

Ilk REPL davranisi:

- Tek satir sistem cagrisi kabul eder.
- Cok satirli `task "name" { ... }` bloklarini tamamlanana kadar toplar.
- Parse sonucunu veya hata mesajini dondurur.
- Runtime henuz hazir degilse `parsed, not executed` cevabi verebilir.

REPL host terminal programi degildir. HNMos icindeki HNShell/console akisi uzerinden calismasi hedeflenir.

## Runtime Plani

HNLang runtime, AST veya task IR'i calistirir.

Ilk runtime sorumluluklari:

- Task adini kaydetmek.
- Komut listesini sirali islemek.
- `read`, `summarize`, `output` gibi task adimlarini Task Engine'e cevirmek.
- `kernel.inspect(...)` gibi cagrilari syscall-backed Core OS servislerine yonlendirmek.
- `ai.ask(...)` gibi cagrilari AI Runtime Interface'e izinli istek olarak aktarmak.
- `shell.run(...)` cagrilarini HNShell komut dispatcher'ina draft veya request olarak iletmek.

Runtime, kernel fonksiyonlarini dogrudan cagiramaz.

## HNShell Entegrasyonu

HNShell, HNLang'i uc sekilde kullanabilir:

- `hnlang` REPL oturumu baslatmak.
- `hn run <script>` benzeri bir komutla task script calistirmak.
- AI tarafindan uretilen HNLang draft'ini kullaniciya gostermek.

Ilk entegrasyon hedefi:

```text
hn lang repl
hn lang parse <inline-script>
hn lang run <task-name>
```

Bu komutlar CODEX-07'de tasarim seviyesindedir; uygulama sonraki kademeye birakilabilir.

## AI Task Script Mantigi

HNLang, AI'nin sistem uzerinde dogrudan komut calistirmasi icin degil, AI onerilerini denetlenebilir task script'lerine cevirmek icin kullanilir.

Dogru akis:

```text
AI proposal
  -> HNLang draft
  -> Safety Layer check
  -> HNShell visible review
  -> user or policy approval
  -> HNLang runtime
  -> syscall-backed Core OS service
  -> audit/event log
```

AI tarafindan uretilen HNLang kodu varsayilan olarak `Draft` seviyesindedir. Otomatik execute edilmez.

## Kernel/User-Space Sinirlari

HNLang user-space hedeflidir.

Kurallar:

- HNLang kernel memory okuyamaz.
- HNLang kernel pointer kullanamaz.
- HNLang fiziksel bellek veya page table degistiremez.
- HNLang kernel servislerine syscall veya Core OS servis arayuzu ile erisir.
- HNLang AI Runtime Interface'e Safety Layer ve Task Engine uzerinden erisir.
- Kernel panic, interrupt, scheduler ve memory manager HNLang tarafindan dogrudan kontrol edilmez.

`kernel.inspect(memory)` gibi ifadeler dogrudan kernel fonksiyonu degil, izinli ve sinirli sistem sorgusu anlamina gelir.

## Ilk Minimal Yorumlayici Plani

Ilk yorumlayici su kapsami hedeflemelidir:

1. String literal ve identifier lexer.
2. `task "name" { ... }` parser.
3. Basit system call expression parser.
4. AST yerine kucuk task IR.
5. `parse only` modunda serial/console cikisi.
6. Runtime henuz yoksa task'i calistirmadan plan olarak gostermek.

## Tamamlanma Kriterleri

- `lang/hnlang/`, `lang/hnlang/parser/`, `lang/hnlang/repl/`, `lang/hnlang/runtime/` klasorleri hazirdir.
- HNLang'in host script dili olmayacagi belirtilmistir.
- HNLang amaci ve HNMos icindeki konumu yazilmistir.
- Parser, REPL ve runtime plani tanimlanmistir.
- HNShell, Task Engine ve AI Runtime Interface entegrasyonu tanimlanmistir.
- Kernel/user-space sinirlari belirtilmistir.
- Ilk ornek programlar syntax belgesinde verilmistir.

## HNMOS-CODEX-08 Knowledge FS Baglantisi

CODEX-08, HNLang task script'lerinin Knowledge FS ile nasil calisacagini tasarlar:

1. `read "/path"` adimi FS metadata ve izin kontrolune baglanir.
2. `summarize` adimi Knowledge FS ozet task'i olarak yorumlanir.
3. `output "file"` adimi permission/audit olmadan kalici yazma yapmaz.
4. `ai.ask(...)` Knowledge FS baglamini Context Router uzerinden alir.
5. Runtime execute etmeyip task planini gorunur tutar.

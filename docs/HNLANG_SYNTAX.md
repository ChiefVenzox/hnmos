# HNLang Syntax

## Hedef

Bu belge, HNLang icin ilk syntax tasarimini tanimlar.

HNLang, HNMos gorevlerini okunabilir, denetlenebilir ve AI Runtime Interface ile uyumlu bicimde ifade etmeyi hedefler.

## Temel Program Yapisi

Ilk ana yapi `task` blokudur:

```hnlang
task "scan files" {
  read "/system"
  summarize
  output "report.md"
}
```

Bu blok su anlama gelir:

- Gorev adi: `scan files`
- Ilk adim: `/system` kaynagini oku.
- Ikinci adim: okunan icerigi ozetle.
- Ucuncu adim: sonucu `report.md` olarak yaz.

Bu adimlar dogrudan kernel fonksiyonu degildir. Task Engine tarafindan izinli sistem servislerine cevrilir.

## Ilk Syntax Ilkeleri

HNLang syntax'i:

- Okunabilir olmalidir.
- Gorev bloklarini birinci sinif kavram yapmalidir.
- Sistem cagrilarini acik namespace ile yazmalidir.
- AI cagrilarini kernel cagrilarindan ayirmalidir.
- Shell komutlarini gorunur string olarak tutmalidir.
- Otomatik execution yerine plan/draft akisini desteklemelidir.

## Yorumlar

Ilk yorum bicimi:

```hnlang
# this is a comment
```

Yorumlar lexer tarafindan yok sayilir.

## String Literal

String literal cift tirnak kullanir:

```hnlang
"report.md"
"Bu bellek durumu normal mi?"
```

Ilk asamada escape dizileri minimum tutulabilir:

- `\"`
- `\\`
- `\n`

## Identifier

Identifier ornekleri:

```hnlang
read
summarize
output
kernel
inspect
memory
ai
ask
shell
run
```

Ilk kural:

```text
[a-zA-Z_][a-zA-Z0-9_]*
```

## Task Bloklari

Syntax:

```hnlang
task "task name" {
  statement
  statement
}
```

Task bloklari Task Engine'e aktarilacak ana yapidir.

## Built-in Task Adimlari

Ilk adimlar:

```hnlang
read "/path"
summarize
output "path"
```

Bu adimlar runtime tarafindan task IR'e cevrilir.

Ilk IR ornegi:

```text
Task(name="scan files")
  Step(read, "/system")
  Step(summarize)
  Step(output, "report.md")
```

## Sistem Cagrisi Syntax'i

Ornek:

```hnlang
kernel.inspect(memory)
ai.ask("Bu bellek durumu normal mi?")
shell.run("hn status")
```

### kernel.inspect(memory)

Kernel durumunu sinirli ve izinli sekilde sorgular.

Bu ifade kernel fonksiyonunu dogrudan cagirmak anlamina gelmez. HNLang runtime bunu Core OS servis istegine cevirir.

### ai.ask(...)

AI Runtime Interface'e soru veya ozetleme istegi gonderir.

Bu ifade:

- Context Router'dan izinli baglam ister.
- Safety Layer tarafindan siniflandirilir.
- Gercek model yoksa stub adapter cevabi alabilir.
- Sistem komutu calistirmaz.

### shell.run(...)

HNShell komutunu calistirma istegidir.

Ilk asamada bu ifade `Draft` veya `Request` seviyesinde degerlendirilmelidir. AI tarafindan uretilirse otomatik execute edilmez.

## Ilk Ornek Programlar

### Sistem Durumu Ozeti

```hnlang
task "system status report" {
  shell.run("hn status")
  shell.run("hn build info")
  output "status.md"
}
```

### Bellek Inceleme

```hnlang
task "inspect memory" {
  kernel.inspect(memory)
  ai.ask("Bu bellek durumu normal mi?")
  output "memory-report.md"
}
```

### Event Ozeti

```hnlang
task "event summary" {
  shell.run("hn event list")
  summarize
  output "events.md"
}
```

### AI Yardimli Ama Execute Etmeyen Plan

```hnlang
task "plan cleanup" {
  read "/system/tmp"
  ai.ask("Silinmesi guvenli dosyalari oner.")
  output "cleanup-plan.md"
}
```

Bu program dosya silmez. Sadece plan uretir.

## Parser Plani

Ilk parser su grammar'i hedefleyebilir:

```text
program       = task_decl*
task_decl     = "task" string "{" statement* "}"
statement     = simple_step | call_expr
simple_step   = identifier string?
call_expr     = identifier "." identifier "(" argument_list? ")"
argument_list = argument ("," argument)*
argument      = string | identifier | number
```

Ilk parser hata mesajlari:

```text
hnlang: expected task name
hnlang: expected '{'
hnlang: unknown statement
hnlang: unterminated string
```

## REPL Plani

REPL iki modda calisabilir:

### Tek Satir Modu

```hnlang
hnlang> kernel.inspect(memory)
parsed: call kernel.inspect(memory)
```

### Task Blok Modu

```hnlang
hnlang> task "scan files" {
......>   read "/system"
......>   summarize
......> }
parsed: task scan files
```

REPL, host terminal prototipi olarak degil, HNMos icinde HNShell tarafindan acilan dil oturumu olarak hedeflenir.

## Runtime Plani

Ilk runtime execute etmeyebilir. Guvenli ilk hedef `parse only` veya `plan only` modudur.

Runtime asamalari:

1. Parse.
2. Task IR olustur.
3. Safety Layer'a task sinifi sor.
4. Gerekirse HNShell'e onay/draft goster.
5. Sadece izinli Core OS servislerine syscall/event interface uzerinden git.

## Kernel/User-Space Sinirlari

HNLang:

- Kernel icinde model veya script motoru olarak calismaz.
- Kernel memory'yi okuyamaz.
- Kernel fonksiyonlarini dogrudan cagiramaz.
- `kernel.*` namespace'ini sinirli sistem sorgulari icin kullanir.
- AI tarafindan uretilen kodu otomatik calistirmaz.

## Tamamlanma Kriterleri

- Task blok syntax'i tanimlanmistir.
- Sistem cagrisi syntax'i tanimlanmistir.
- Parser grammar taslagi yazilmistir.
- REPL davranisi planlanmistir.
- Ilk ornek programlar eklenmistir.
- Kernel/user-space sinirlari belirtilmistir.

## HNMOS-CODEX-08 Knowledge FS Baglantisi

CODEX-08'de syntax planinin Knowledge FS tarafindan anlamlandirilmasi hedeflenir:

1. `read` adimlari metadata ve izin sorgusuna cevrilir.
2. `summarize` adimlari ozet task'i olarak isaretlenir.
3. `output` adimlari kalici yazma onayi gerektirir.
4. `kernel.inspect(memory)` guvenli metadata/sistem sorgusu olarak sinirlanir.
5. AI tarafindan uretilen HNLang kodu otomatik execute edilmez.

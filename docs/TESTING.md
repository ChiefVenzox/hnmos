# Testing

## Hedef

HNMOS-CODEX-13'un hedefi, Kernel 01, HNShell, AI Runtime Interface, HNLang ve Knowledge FS parcalarinin kirilmadan gelisebilmesi icin test, debug ve CI stratejisini tasarlamaktir.

HNMos test sistemi host OS uzerinde calisan bir HNMos prototipi degildir. Host sadece compiler, linker, statik dogrulama araclari, QEMU ve CI runner ortami olarak kullanilir.

## Dosya Yapisi

```text
tests/
|-- kernel/
|-- shell/
|-- ai/
|-- hnlang/
`-- knowledge_fs/

tools/
`-- test/
    |-- kernel-abi-check.sh
    |-- shell-compat.sh
    |-- static-check.sh
    `-- qemu-smoke.sh

.github/
`-- workflows/
    `-- hnmos-ci.yml

docs/
|-- TESTING.md
`-- DEBUGGING.md
```

- `tests/kernel/`: kernel unit, boot, panic ve low-level behavior test hedefleri.
- `tests/shell/`: HNShell parser ve komut davranisi test hedefleri.
- `tests/ai/`: AI Runtime Interface, permission ve task proposal test hedefleri.
- `tests/hnlang/`: lexer/parser/AST ve REPL davranisi test hedefleri.
- `tests/knowledge_fs/`: metadata, tag, summary ve index test hedefleri.
- `tools/test/`: host tarafinda calisan test yardimcilari.
- `.github/workflows/`: CI workflow ornekleri.

## Test Stratejisi

Testler kademeli olmalidir:

1. Statik build dogrulamasi.
2. Unit tests.
3. Kernel boot tests.
4. QEMU smoke tests.
5. Shell command tests.
6. AI Runtime Interface tests.
7. HNLang parser tests.
8. Knowledge FS metadata tests.
9. Release oncesi tam kontrol listesi.

Ilk kural:

```text
Kernel boot etmiyorsa ust katman testleri guvenilir kabul edilmez.
```

## Unit Tests

Unit test hedefleri host uygulamasi gibi HNMos calistirmak degil, saf veri yapilarini ve parser/policy mantigini izole kontrol etmektir.

Ilk unit test adaylari:

- HNShell command tokenizer.
- HNLang lexer/parser.
- Permission manifest parser.
- Policy decision matrix.
- Knowledge FS metadata struct validation.
- App manifest parser.

Uzun vadede unit testler HNMos SDK icinde veya host-side freestanding test runner ile calistirilabilir.

## Kernel Boot Tests

Kernel boot testleri Kernel 01'in build edildigini, Multiboot uyumlu oldugunu ve beklenen acilis mesajlarini verdigini kontrol eder.

Statik boot kontrolleri:

```sh
make kernel
make verify
```

Beklenen statik sonuc:

```text
kernel ABI: ELF32 i686, Multiboot, assembly symbols and no-SIMD policy ok
```

Checker ELF header/entry, Multiboot header/checksum, undefined symbol, zorunlu assembly sembolleri ve sync talimatlari ile final-binary no-SIMD/FPU politikasini denetler. Bu statik kontrol tek basina spinlock stress/correctness testi veya gercek GPU completion kaniti degildir.

## QEMU Boot Test Mantigi

QEMU smoke test, kernelin gercek emulator icinde acilip serial log uzerinden beklenen mesajlari vermesini kontrol eder.

Komut:

```sh
sh tools/test/qemu-smoke.sh
```

Beklenen serial log:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
status: Kernel 01 is idle.
```

Kernel sonsuz `hlt` dongusunde kalabildigi icin QEMU smoke test timeout ile sonlandirilir. Timeout, mesajlar gorulduyse hata kabul edilmez.

## Host Shell Uyumlulugu

```sh
make check-shell
```

Bu hedef repo icindeki `.sh` dosyalarinin exact POSIX shebang'ini, `sh -n`, `bash --posix -n` ve varsa `dash -n` parse sonucunu kontrol eder. Ayrica build/QEMU dispatcher'larinin repo disindaki bir current directory'den gecersiz arguman davranisini bu shell'ler altinda dogrular. Bu test host build/test orkestrasyonu icindir; kernel icindeki HNShell parser veya runtime testi degildir.

## Kernel Panic Test Mantigi

Panic testi, kernelin hatayi sessizce yutmamasini ve panic mesajini VGA/serial uzerinden yazmasini kontrol eder.

Ilk panic senaryolari:

- Invalid Multiboot magic.
- Null pointer guard test hedefi.
- Double fault simule hedefi.
- Fatal driver init failure.

Beklenen panic ciktisi:

```text
HNMos kernel panic.
reason: <message>
```

Panic testleri normal boot testlerinden ayri kosulmalidir. CI icinde `panic-test` build flag'i veya ayrilmis test kernel target'i kullanilmalidir.

## Shell Command Tests

HNShell testleri once parser ve routing seviyesinde baslamalidir.

Ilk test komutlari:

```text
hn status
hn event list
hn ai status
hn app list
hn app info hn-monitor
```

Beklenen test katmanlari:

- Tokenizer input/output.
- Command parser.
- Built-in command dispatch.
- Task Router output.
- Permission preview output.

Shell testleri kernel console'a bagimli olmadan parser seviyesinde unit test olarak baslayabilir; sonra QEMU console testine tasinir.

## AI Runtime Interface Tests

AI testleri gercek LLM calistirmak zorunda degildir. Ilk asamada stub adapter ve policy karar testleri yeterlidir.

Test hedefleri:

- AI action proposal parse.
- Context Router izin filtresi.
- Permission manifest olusturma.
- Policy Engine karar matrisi.
- Safety Layer forbidden action deny.
- Audit event generation.

Ornek:

```text
input: AI proposes delete.fs.file /logs/*
expected: risk=critical, decision=require_user_approval
```

## HNLang Parser Tests

HNLang testleri dilin gorev odakli syntax'ini korur.

Ornek test girdisi:

```text
task "scan files" {
  read "/system"
  summarize
  output "report.md"
}
```

Beklenen:

```text
AST task.name = scan files
AST actions = read, summarize, output
permission hints = read.fs.content, write.fs.file
```

Ilk testler:

- Token listesi.
- AST node tipi.
- Syntax error konumu.
- Kernel/user-space yasak cagri algilama.

## Knowledge FS Metadata Tests

Knowledge FS testleri dosya icerigi yerine metadata sozlesmesini dogrular.

Test hedefleri:

- Metadata record parse.
- Tag list validation.
- Summary field limit.
- Permission class validation.
- Index key generation.
- AI-visible vs sensitive metadata ayrimi.

Ornek:

```json
{
  "path": "/system/config",
  "tags": ["system", "config"],
  "summary": "Boot configuration metadata.",
  "ai_visible": true
}
```

## CI Workflow Ornegi

CI iki temel isle baslar:

1. `kernel-static`: toolchain kurar, ABI/no-SIMD ve POSIX/Bash shell kontrollerini calistirir, manifest JSON'larini dogrular.
2. `qemu-smoke`: Kernel 01'i QEMU'da boot eder ve serial log mesajlarini arar.

Workflow dosyasi:

```text
.github/workflows/hnmos-ci.yml
```

CI icin minimum komutlar:

```sh
sh tools/test/static-check.sh
sh tools/test/qemu-smoke.sh
```

## Release Oncesi Kontrol Listesi

Release adayi cikmadan once:

- `make clean`
- `make kernel`
- `make verify`
- Manifest JSON dogrulamasi.
- QEMU kernel smoke test.
- ISO image build, araclar varsa.
- QEMU ISO boot smoke test, araclar varsa.
- Panic test target'i, hazirsa.
- Shell command parser tests.
- AI permission matrix tests.
- HNLang parser tests.
- Knowledge FS metadata tests.
- Docs guncelleme kontrolu.

## Tamamlanma Kriterleri

- `tests/kernel/`, `tests/shell/`, `tests/ai/`, `tests/hnlang/` ve `tests/knowledge_fs/` alanlari hazirdir.
- `tools/test/` altinda statik dogrulama ve QEMU smoke test yardimcilari hazirdir.
- `.github/workflows/hnmos-ci.yml` CI ornegi eklenmistir.
- Test stratejisi tanimlanmistir.
- QEMU boot test mantigi yazilmistir.
- Kernel panic test mantigi yazilmistir.
- Shell, AI Runtime Interface, HNLang ve Knowledge FS test hedefleri tanimlanmistir.
- Release oncesi kontrol listesi hazirlanmistir.

## Test Uygulama Notu

Sonraki test hedefleri:

1. Mevcut `make check` hedefini unit testlerle genisletmek.
2. Mevcut `sh tools/test/qemu-smoke.sh` akisina HNShell input otomasyonu eklemek.
3. HNShell parser icin ilk unit test taslagi.
4. HNLang parser fixture dizini.
5. AI permission matrix test verisi.
6. Knowledge FS metadata fixture verisi.

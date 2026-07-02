# Debugging

## Hedef

Bu belge, HNMos Kernel 01 ve ust katmanlar icin debug log, QEMU inceleme ve hata ayiklama yaklasimini tanimlar.

HNMos debug sistemi, host uzerinde calisan bir uygulama debugger'i degildir. Host sadece QEMU, serial output, ELF araclari ve CI loglari icin kullanilir.

## Debug Katmanlari

Debug akisi katmanli olmalidir:

1. Build-time diagnostics.
2. Static ELF/Multiboot verification.
3. Serial log.
4. VGA text output.
5. QEMU smoke log.
6. Kernel panic output.
7. Event/audit ring buffer.
8. User-space service logs.

## Debug Log Sistemi

Kernel 01 icin ilk log kanallari:

```text
VGA text output
COM1 serial output
```

Mevcut beklenen boot log:

```text
HNMos Kernel 01 initialized.
AI-native operating layer will be loaded later.
boot mode: bare-metal kernel
status: Kernel 01 is idle.
```

Log seviyeleri uzun vadede:

```text
trace
debug
info
warn
error
panic
audit
```

Ilk Kernel 01 icin `info`, `error`, `panic` ve `audit` yeterlidir.

## QEMU Debug Mantigi

Temel QEMU boot:

```sh
sh tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

Smoke test:

```sh
sh tools/test/qemu-smoke.sh
```

GDB hedefli ilerideki QEMU modlari:

```text
qemu-system-i386 -S -s -kernel build/hnmos-kernel01.elf
```

- `-S`: CPU'yu baslangicta durdurur.
- `-s`: `tcp::1234` GDB server acar.

Bu mod CODEX-13'te sadece tasarlanir; aktif GDB otomasyonu sonraki kademede eklenebilir.

## Kernel Panic Debug Mantigi

Panic ciktisi her zaman su bilgileri tasimalidir:

```text
HNMos kernel panic.
reason: <message>
last_event_id: <id>
last_task_id: <id>
tick: <tick>
```

Kernel 01 su anda temel panic mesajini basar. Ileride panic sistemine event id, task id ve audit tail eklenecektir.

Panic testleri normal boot ile karistirilmamalidir. Panic beklenen bir testte QEMU'nun kapanmamasi hata degildir; serial log icindeki panic metni basari kriteridir.

## Shell Debug

HNShell debug hedefleri:

- Raw input.
- Token listesi.
- Parser sonucu.
- Command dispatch sonucu.
- Task Router output.
- Permission preview.

Ornek debug ciktisi:

```text
shell.debug input="hn app info hn-monitor"
shell.debug tokens=[hn, app, info, hn-monitor]
shell.debug command=app.info target=hn-monitor
```

Bu ciktinin varsayilan release modunda kapali olmasi gerekir.

## AI Runtime Interface Debug

AI debug loglari model dusuncesi gibi gizli veya cok genis icerik tasimamalidir. Sadece OS acisindan denetlenebilir karar ozeti tutulur.

Loglanacak alanlar:

- Task id.
- Actor.
- Requested context.
- Sanitized context scope.
- Proposed action.
- Policy decision.
- User approval state.
- Audit event id.

Loglanmayacak alanlar:

- Sensitive file content.
- Secret payload.
- Raw microphone/camera stream.
- Kernel memory dump.
- Kullanici onayi olmayan private context.

## HNLang Debug

HNLang icin debug hedefleri:

- Token stream.
- AST tree.
- Permission hints.
- Runtime action plan.
- Syntax error location.

Ornek:

```text
hnlang.debug token_count=7
hnlang.debug ast.root=task
hnlang.debug permission_hint=read.fs.content
```

## Knowledge FS Debug

Knowledge FS debug hedefleri:

- Metadata parse sonucu.
- Summary length.
- Tag normalization.
- AI visibility class.
- Index key.
- Permission class.

Sensitive dosya icerigi debug log'a yazilmaz.

## CI Debug

CI hata ayiklama sirasinda incelenecekler:

1. Toolchain kurulum logu.
2. `make kernel` compiler hatasi.
3. `make verify` ELF/Multiboot sonucu.
4. Manifest JSON parse hatasi.
5. QEMU serial log.
6. Timeout durumu.

QEMU smoke test basarisiz olursa CI logunda serial cikti gorunmelidir.

## Release Oncesi Debug Kontrolu

Release adayi oncesi:

- Panic mesaji okunabilir mi?
- Serial log QEMU'da gorunuyor mu?
- VGA log QEMU ekraninda gorunuyor mu?
- `make verify` Multiboot sonucunu gosteriyor mu?
- QEMU smoke timeout beklenen sekilde mi?
- AI permission deny kararlari audit log'a dusuyor mu?
- Shell parser hatalari kullaniciya temiz mesaj veriyor mu?
- HNLang syntax error konumu raporlaniyor mu?

## Tamamlanma Kriterleri

- Debug log katmanlari tanimlanmistir.
- Serial ve VGA log rolleri aciklanmistir.
- QEMU debug ve smoke test mantigi yazilmistir.
- Kernel panic debug beklentisi belirtilmistir.
- Shell, AI Runtime Interface, HNLang ve Knowledge FS debug hedefleri yazilmistir.
- CI debug kontrol sirasi belirlenmistir.

## Debug Uygulama Notu

Debug tasarimi ilk uygulanabilir araclara indirilirken hedef:

1. `tools/test/qemu-smoke.sh` Makefile hedefi.
2. CI artifact olarak QEMU serial log saklama.
3. Kernel panic test build flag'i.
4. Shell parser debug fixture'i.
5. HNLang parser debug fixture'i.
6. Audit/debug ring buffer taslagi.

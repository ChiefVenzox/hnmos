# Repo Structure

## Hedef

Bu belge, `hnmos` reposunun gercek bare-metal isletim sistemi gelistirme yapisini tanimlar.

## Ana Yapi

```text
hnmos/
|-- RELEASE_NOTES.md
|-- docs/
|-- kernel01/
|-- shell/
|-- ai/
|-- lang/
|-- ui/
|-- sdk/
|-- tools/
|-- examples/
|-- scripts/
`-- tests/
```

## Klasorlerin Amaci

### docs/

Mimari, build, toolchain, boot image ve katkida bulunma belgeleri burada tutulur.

CODEX-14 belgeleri:

- `DEMO.md`: v0.0.1 bare-metal demo senaryosu.
- `VERSION_0_0_1.md`: v0.0.1 kapsam, manifesto ve roadmap.

### RELEASE_NOTES.md

HNMos surum notlari burada tutulur. v0.0.1 ilk demo surumunu tanimlar.

### kernel01/

Ilk boot eden HNMos cekirdegi burada gelisir.

```text
kernel01/
|-- boot/
|-- kernel/
|-- console/
|-- memory/
|-- interrupts/
|-- timer/
|-- scheduler/
|-- syscalls/
|-- events/
|-- drivers/
`-- fs/
    `-- metadata/
```

- `boot/`: Multiboot header, `_start`, erken stack ve boot gecisi.
- `kernel/`: `main.c`, `log.*`, `panic.*` ve cekirdek yardimcilari.
- `console/`: erken kernel console, prompt, line editor ve console I/O tasarimi.
- `memory/`: fiziksel bellek, kernel heap ve paging icin ayrilan alan.
- `interrupts/`: GDT, IDT, exception, IRQ ve timer hazirligi.
- `timer/`: PIT, tick sayaci ve timer interrupt altyapisi.
- `scheduler/`: kernel task modeli ve round-robin scheduler hazirligi.
- `syscalls/`: uzun vadeli user-space syscall kapisi.
- `events/`: driver olaylarini standart kernel event queue kayitlarina ceviren alan.
- `drivers/`: VGA, klavye, disk ve driver event kaynaklari.
- `fs/`: initramfs, basit FS ve Knowledge FS'e giden cekirdek tarafi.
- `fs/metadata/`: kernel seviyesinde guvenli dosya metadata modeli.

### shell/

HNShell burada gelisir. HNShell, uzun vadede user-space veya kontrollu sistem servisi olarak calisir. Kernel icine kalici komut listesi olarak gomulmez.

CODEX-05 yapisi:

```text
shell/
|-- hnshell/
|-- parser/
|-- commands/
|   `-- events/
`-- task_router/
```

- `hnshell/`: oturum ve runtime cekirdegi.
- `parser/`: komut satiri token/parser yapisi.
- `commands/`: built-in `hn ...` komutlari.
- `commands/events/`: `hn event ...` komutlari.
- `task_router/`: komutlari task request modeline ceviren gecis katmani.

### ai/

AI Runtime Interface burada gelisir. AI kernel icine gomulmez; olay, gorev, izin, baglam ve audit arayuzleri uzerinden OS ile etkilesir.

CODEX-06 yapisi:

```text
ai/
|-- runtime_interface/
|-- task_engine/
|-- context_router/
|-- safety_layer/
|-- permission_manifest/
|-- action_audit/
|-- policy_engine/
|-- model_runtime_spec/
|-- memory_index/
|-- semantic_fs/
|-- indexer/
`-- event_reader/
```

- `runtime_interface/`: HNShell, Task Engine ve model runtime arasindaki AI servis kapisi.
- `task_engine/`: komutlari izlenebilir task request/result kayitlarina ceviren katman.
- `context_router/`: modele verilecek izinli baglami sinirlayan katman.
- `safety_layer/`: risk, izin ve execution politikasini uygulayan katman.
- `permission_manifest/`: AI, HNShell ve HNLang task izin tanimlari.
- `action_audit/`: kritik eylem, kullanici onayi ve policy karar kayitlari.
- `policy_engine/`: capability, risk ve approval kararlarini ureten katman.
- `model_runtime_spec/`: gelecekteki local model runtime adapter sozlesmesi.
- `memory_index/`: Knowledge FS arama ve semantic memory index hedefi.
- `semantic_fs/`: AI Runtime Interface icin semantic FS sorgu arayuzu.
- `indexer/`: metadata, ozet ve etiketlerden index uretecek user-space hedefi.
- `event_reader/`: AI icin izinli ve sanitize edilmis event snapshot okuyucu.

### lang/

HNLang kaynaklari burada gelisir. HNLang, HNShell ve AI tarafindan uretilen gorev planlarini denetlenebilir sistem betiklerine cevirecek dil katmanidir.

CODEX-07 yapisi:

```text
lang/
`-- hnlang/
    |-- parser/
    |-- repl/
    `-- runtime/
```

- `hnlang/`: HNLang ana dil cekirdegi.
- `hnlang/parser/`: lexer, parser, token ve AST/IR hedefleri.
- `hnlang/repl/`: HNShell icinden calisacak minimal REPL hedefi.
- `hnlang/runtime/`: task IR'i Task Engine ve Core OS servislerine yonlendirecek runtime hedefi.

### ui/

HNMos'un kendi OS UI deneyleri icin ayrilir. Bu klasor host desktop uygulamasi hedefi degildir.

CODEX-10 yapisi:

```text
ui/
|-- console/
|-- framebuffer/
|-- window_manager/
|-- command_palette/
|-- assistant_panel/
`-- task_monitor/
```

- `console/`: erken text-mode UI, HNShell prompt ve status line hedefi.
- `framebuffer/`: pixel tabanli cizim, font render ve framebuffer console hedefi.
- `window_manager/`: uzun vadeli pencere, focus ve compositor modeli.
- `command_palette/`: gorev odakli kullanici niyet girisi.
- `assistant_panel/`: AI Runtime Interface icin guvenli panel yuzeyi.
- `task_monitor/`: Visual Task Monitor icin gorev, event ve izin durumu gorunumu.

### sdk/

HNMos uzerinde calisacak sistem araclari, gorev uygulamalari ve AI destekli yardimci uygulamalar icin SDK tasarim alani.

CODEX-12 yapisi:

```text
sdk/
|-- templates/
`-- examples/
```

- `templates/`: app manifest ve uygulama sablonlari.
- `examples/`: SDK tasariminda referans alinacak genel ornekler.

### tools/

Host uzerinde calisan gelistirme araclari burada tutulur.

```text
tools/
|-- toolchain/
|-- qemu/
|-- image/
`-- test/
```

- `toolchain/`: compiler ve arac kontrol scriptleri.
- `qemu/`: QEMU calistirma yardimcilari.
- `image/`: boot edilebilir ISO/disk imaji uretim yardimcilari.
- `test/`: statik dogrulama, QEMU smoke ve CI test yardimcilari.

### examples/

HNShell komutlari, HNLang betikleri, SDK uygulamalari ve sistem davranisi ornekleri burada tutulur.

CODEX-12 yapisi:

```text
examples/
|-- hn-notes/
|-- hn-monitor/
|-- hn-ai-lab/
`-- demo_task.hn
```

- `hn-notes/`: not, metadata ve ozetleme odakli app hedefi.
- `hn-monitor/`: sistem status, event snapshot ve task monitor app hedefi.
- `hn-ai-lab/`: AI Runtime Interface icin guvenli deney app hedefi.
- `demo_task.hn`: v0.0.1 demo gorev ornegi.

### scripts/

Demo ve release yardimci scriptleri burada tutulur.

- `demo.sh`: v0.0.1 icin build, verify ve QEMU demo yardimcisi.

### tests/

Build, imaj, statik ELF dogrulama, emulator tabanli testler ve ust katman test hedefleri burada tutulur.

CODEX-13 yapisi:

```text
tests/
|-- kernel/
|-- shell/
|-- ai/
|-- hnlang/
`-- knowledge_fs/
```

- `kernel/`: kernel boot, panic ve low-level behavior test hedefleri.
- `shell/`: HNShell parser ve komut test hedefleri.
- `ai/`: AI Runtime Interface, permission ve safety test hedefleri.
- `hnlang/`: HNLang lexer, parser ve REPL test hedefleri.
- `knowledge_fs/`: Knowledge FS metadata, tag ve index test hedefleri.

### .github/workflows/

CI workflow ornekleri burada tutulur.

- `hnmos-ci.yml`: kernel static verification ve QEMU smoke test workflow ornegi.

## Tamamlanma Kriteri

- Tum ana klasorler olusturulmustur.
- Kernel 01 alt klasorleri boot, kernel, console, memory, interrupts, timer, scheduler, syscalls, drivers ve fs olarak ayrilmistir.
- Toolchain, QEMU ve image araclari `tools/` altinda ayrilmistir.
- HNShell, AI Runtime Interface ve HNLang icin kalici kaynak alanlari ayrilmistir.

## HNMOS-CODEX-06 Durumu

CODEX-06, bu repo yapisi uzerinde `ai/` alanlarini AI Runtime Interface, Task Engine, Context Router, Safety Layer ve model runtime spec olarak ayirmistir.

## HNMOS-CODEX-07 Durumu

CODEX-07, bu repo yapisi uzerinde `lang/hnlang/` alanlarini HNLang parser, REPL ve runtime hedefleri olarak ayirmistir.

## HNMOS-CODEX-08 Durumu

CODEX-08, bu repo yapisi uzerinde Knowledge FS icin `kernel01/fs/metadata/`, `ai/memory_index/`, `ai/semantic_fs/` ve `ai/indexer/` alanlarini ayirmistir.

## HNMOS-CODEX-09 Durumu

CODEX-09, bu repo yapisi uzerinde Driver Event Layer icin `kernel01/events/`, `ai/event_reader/` ve `shell/commands/events/` alanlarini ayirmistir.

## HNMOS-CODEX-10 Durumu

CODEX-10, bu repo yapisi uzerinde UI Layer icin `ui/console/`, `ui/framebuffer/`, `ui/window_manager/`, `ui/command_palette/`, `ui/assistant_panel/` ve `ui/task_monitor/` alanlarini ayirmistir.

## HNMOS-CODEX-11 Durumu

CODEX-11, bu repo yapisi uzerinde AI Permission Layer icin `ai/safety_layer/`, `ai/permission_manifest/`, `ai/action_audit/` ve `ai/policy_engine/` alanlarini ayirmistir.

## HNMOS-CODEX-12 Durumu

CODEX-12, bu repo yapisi uzerinde SDK ve uygulama modeli icin `sdk/`, `sdk/templates/`, `sdk/examples/`, `examples/hn-notes/`, `examples/hn-monitor/` ve `examples/hn-ai-lab/` alanlarini ayirmistir.

## HNMOS-CODEX-13 Durumu

CODEX-13, bu repo yapisi uzerinde test/debug/CI sistemi icin `tests/kernel/`, `tests/shell/`, `tests/ai/`, `tests/hnlang/`, `tests/knowledge_fs/`, `tools/test/` ve `.github/workflows/` alanlarini ayirmistir.

## HNMOS-CODEX-14 Durumu

CODEX-14, HNMos v0.0.1 ilk demo surumu icin `RELEASE_NOTES.md`, `docs/DEMO.md`, `docs/VERSION_0_0_1.md`, `examples/demo_task.hn` ve `scripts/demo.sh` dosyalarini hazirlamistir.

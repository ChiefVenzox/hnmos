# HNMos System Layers

## Hedef

Bu belge, HNMos'un gelistirme modelini dort ana katman halinde tanimlar. Amac, host ortamda calisan araclarla HNMos'un gercek OS parcasi olan kodlari kesin bicimde ayirmaktir.

## Dosya Yapisi

```text
docs/
|-- SYSTEM_LAYERS.md
|-- ARCHITECTURE.md
|-- KERNEL_01_OVERVIEW.md
`-- AI_NATIVE_INTERFACE.md

tools/
`-- run-qemu.sh

kernel/
|-- arch/
`-- core/
```

## 1. Toolchain Layer

Toolchain Layer, HNMos'un uretim katmanidir. Bu katman Linux/macOS/Windows uzerinde calisabilir; cunku HNMos'un kendisi degil, HNMos'u ureten araclar burada bulunur.

Bu katmanda izin verilen host kullanimlari:

- Compiler calistirma.
- Assembler calistirma.
- Linker calistirma.
- ISO veya disk image uretme.
- QEMU baslatma.
- Build script ve test script calistirma.

Bu katmanda izin verilmeyen yaklasimlar:

- HNMos'u bir desktop uygulamasi gibi modellemek.
- HNShell'i host terminal uygulamasi olarak asil hedefe cevirmek.
- AI runtime'i host uygulamasi olarak HNMos'un yerine gecirmek.
- Dosya sistemi, scheduler veya driver event modelini host API'leriyle kalici tasarlamak.

Toolchain ciktisi kernel ELF, ISO veya disk imajidir. Hedef sistem bu imajlardan boot eder.

## 2. Bare-Metal Kernel Layer

Bare-Metal Kernel Layer, QEMU veya donanim tarafindan yuklenen ilk HNMos kodudur.

Bu katmanin ilk hedefi Kernel 01'dir:

- Multiboot uyumlu kernel ELF.
- x86 protected mode baslangici.
- Kendi stack alani.
- Linker script ile kontrollu bellek yerlesimi.
- VGA text buffer uzerinden dogrudan cikis.
- Host OS API'siz calisma.

Bu katmanda kernel kucuk tutulur. Erken evrede sadece boot, panic, temel cikis, descriptor tablolar, interrupt ve donanim gecisleri hedeflenir.

## 3. Core OS Layer

Core OS Layer, cekirdegin uzerine sistem davranisini kurar.

Ana bilesenler:

- Memory: fiziksel bellek haritasi, sayfa ayirici, kernel heap, sanal bellek.
- Interrupt: GDT, IDT, exception, IRQ, interrupt controller.
- Timer: zaman kesmesi, tick sayaci, uyku ve zamanlayici temeli.
- Scheduler: task yapisi, context switch, kernel task ve user task ayrimi.
- Syscall: user-space tarafindan kernel servislerine kontrollu erisim.
- Driver Event: cihaz olaylarini standart OS olaylarina cevirme.
- File System: boot imaji, initramfs, basit dosya yapisi ve Knowledge FS'e gecis.
- Shell: HNShell'in sistemle etkilesim kapisi.

Core OS Layer, AI-native sistem arayuzunun ihtiyac duydugu guvenilir olay ve izin temelini uretir.

## 4. AI-Native System Interface Layer

AI-Native System Interface Layer, HNMos'un AI destekli tarafidir. Bu katman kernel icine gomulu bir model veya prompt sistemi degildir.

Bu katman sunlari tasarlar:

- Sistem olaylarini AI icin okunabilir hale getiren event stream.
- Kullanici isteklerini gorev grafigine ceviren task interface.
- AI eylemlerini izin seviyelerine baglayan permission gate.
- Dosya, shell, cihaz ve gorev baglamini sinirlayan context broker.
- Yapilan onerileri ve eylemleri kaydeden audit log.

AI, dogrudan kernel fonksiyonu cagirmak yerine bu OS arayuzleriyle calisir.

## Katmanlar Arasi Akis

```text
Source Code
  -> Toolchain Layer
  -> Kernel ELF / ISO / Disk Image
  -> QEMU or Hardware
  -> Bare-Metal Kernel Layer
  -> Core OS Layer
  -> AI-Native System Interface Layer
  -> HNShell / HNLang / User Tasks
```

## Kernel/User-Space Ayrimi

Katman modeli, kernel ve user-space ayrimini zorunlu kilar.

Kernel-space:

- Guvenlik sinirini uygular.
- Bellek ve donanim erisimini denetler.
- Syscall ve interrupt kapilarini yonetir.

User-space:

- HNShell'i calistirir.
- HNLang servislerini calistirir.
- AI yardimci servislerini calistirir.
- Kullanici gorevlerini izole eder.

Bu ayrim erken kademede tam uygulanmasa bile mimari hedef olarak korunur.

## Tamamlanma Kriteri

Bu belge tamamlandiginda HNMos'un dort katmanli gelistirme modeli aciktir:

- Host OS sadece toolchain olarak tanimlanmistir.
- Bare-metal kernel hedefi ayrica belirtilmistir.
- Core OS bilesenleri listelenmistir.
- AI-native katman kernel disinda sistem arayuzu olarak konumlanmistir.

## HNMOS-CODEX-02'ye Gecis

CODEX-02'de bu katman modelinin ilk pratik adimi atilmalidir: Bare-Metal Kernel Layer, GDT/IDT ve interrupt hazirligi ile genisletilmelidir.

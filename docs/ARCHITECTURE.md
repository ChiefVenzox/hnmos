# HNMos Architecture

## Hedef

HNMOS-CODEX-01'in hedefi, HNMos icin host prototip mantigindan ayrilmis, QEMU/bare-metal hedefli ana isletim sistemi mimarisini tanimlamaktir.

Bu belge, HNMos'un Linux/macOS/Windows uzerinde calisan gecici bir uygulama degil, sifirdan gelistirilen deneysel bir isletim sistemi oldugunu temel karar olarak kayda gecer.

## Dosya Yapisi

```text
docs/
|-- ARCHITECTURE.md
|-- SYSTEM_LAYERS.md
|-- KERNEL_01_OVERVIEW.md
`-- AI_NATIVE_INTERFACE.md

kernel01/
|-- boot/
|   `-- boot.s
|-- kernel/
|   |-- log.c
|   |-- log.h
|   |-- main.c
|   |-- panic.c
|   |-- panic.h
|   `-- types.h
|-- memory/
|-- interrupts/
|-- drivers/
`-- fs/

linker.ld
```

## Genel Sistem Mimarisi

HNMos dort ana gelistirme katmani uzerinde ilerler:

1. Toolchain Layer
2. Bare-Metal Kernel Layer
3. Core OS Layer
4. AI-Native System Interface Layer

Bu katmanlar bir uygulama yiginini degil, gercek bir isletim sistemi insa yolunu temsil eder. Toolchain host ortamda calisir; HNMos ise QEMU veya uzun vadede gercek donanim uzerinde boot eden hedef sistemdir.

```text
Host OS
  |
  | compiler / assembler / linker / image builder / QEMU launcher
  v
Toolchain Artifacts
  |
  | kernel ELF / ISO / disk image
  v
QEMU or Bare Metal
  |
  v
HNMos Kernel
  |
  | memory / interrupts / timer / scheduler / syscall / drivers
  v
Core OS Services
  |
  | HNShell / HNLang / Knowledge FS / AI system interfaces
  v
User-Space and AI-Native Tasks
```

## Toolchain ile OS Arasindaki Fark

Toolchain, HNMos'un kendisi degildir. Toolchain sadece HNMos'u uretmek ve calistirmak icin kullanilan dis gelistirme ortamidir.

Toolchain Layer sunlari yapar:

- Cekirdek kodunu derler.
- Assembly boot kodunu nesne dosyasina cevirir.
- Linker script ile kernel imajini yerlestirir.
- ISO veya disk imaji uretir.
- QEMU'yu baslatir.

HNMos OS sunlari yapar:

- CPU kontrolunu boot sonrasinda devralir.
- Kendi stack, memory, interrupt ve scheduler modelini kurar.
- Donanim olaylarini kendi driver event modeliyle yorumlar.
- Shell, dil, dosya sistemi ve AI arayuzlerini sistem servisi olarak sunar.

Host OS tarafinda calisan bir HNMos "runtime" veya masaustu uygulamasi bu mimarinin hedefi degildir.

## Kernel ve User-Space Ayrimi

HNMos uzun vadede kernel-space ve user-space ayrimini korur.

Kernel-space sorumluluklari:

- CPU durumunu ve privilege seviyelerini yonetmek.
- Bellek haritalama ve izolasyon saglamak.
- Interrupt ve exception akislarini karsilamak.
- Timer ve scheduler uzerinden gorevleri calistirmak.
- Syscall kapisini denetlemek.
- Surucu olaylarini cekirdek guvenlik siniri icinde toplamak.

User-space sorumluluklari:

- HNShell oturumlari.
- HNLang yorumlayici veya derleyici servisleri.
- Kullanici gorevleri.
- Knowledge FS araclari.
- AI ile etkilesen yardimci servisler.

Bu ayrim, AI dahil olmak uzere yuksek seviye sistem davranislarinin kernel icinde kontrolsuz genislemesini engeller.

## Kernel 01 Hedefi

Kernel 01, HNMos'un ilk boot kanitidir.

Kernel 01 su hedeflere sahiptir:

- QEMU uzerinde boot etmek.
- Freestanding olarak calismak.
- Host OS kutuphanesi kullanmamak.
- Kendi entry point'i ve stack'i ile baslamak.
- VGA text buffer'a dogrudan boot mesaji yazmak.
- Multiboot durum bilgisini gostermek.
- Sistemi kontrollu `hlt` dongusunde tutmak.

Kernel 01 bir shell, dosya sistemi veya AI runtime hedefi degildir. Bu kademe sadece gercek boot zincirini dogrular.

## Core OS Bilesenleri

Core OS Layer, Kernel 01'den sonra asamali olarak su bilesenleri kurar:

- Memory Manager: fiziksel sayfa takibi, kernel heap, sanal bellek, user-space izolasyonu.
- Interrupt Manager: GDT, IDT, exception handler, IRQ yonlendirme.
- Timer: PIT veya APIC tabanli ilk zaman kesmesi.
- Scheduler: ilk task modeli, kernel task'lari, user task'lari.
- Syscall Layer: user-space servis cagri kapisi.
- Driver Event Layer: klavye, disk, ekran ve diger surucu olaylarini standart forma cevirme.
- File System Layer: once initramfs veya basit disk imaji, sonra Knowledge FS temeli.
- Shell Layer: HNShell'in user-space veya kontrollu sistem servisi olarak calismasi.

Core OS Layer, AI-native arayuz icin gerekli olay, izin ve baglam verisini ureten ana sistem tabanidir.

## HNShell'in OS Icindeki Yeri

HNShell, HNMos'un insan ile sistem arasindaki ilk dogrudan komut yuzudur.

HNShell kernel icine gomulu bir komut listesi olarak kalmamali, Core OS olgunlastikca user-space veya sistem servisi olarak calismalidir. Ilk evrelerde debug shell olarak kernel tarafindan baslatilabilir; fakat uzun vadeli mimaride syscall, dosya sistemi ve gorev yonetimi uzerinden sisteme erisir.

HNShell'in gorevi:

- Sistem komutlarini calistirmak.
- HNLang betiklerini tetiklemek.
- Knowledge FS uzerindeki bilgi nesneleriyle calismak.
- AI-native arayuze kullanici niyetini gorev olarak aktarmak.

## HNLang'in OS Icindeki Yeri

HNLang, HNMos'un gorev odakli sistem dili olarak tasarlanir.

HNLang kernel'in parcasi degil, OS uzerinde calisan kontrollu bir dil ortami olmalidir. Kernel yalnizca HNLang'in ihtiyac duydugu temel syscall, dosya, gorev ve izin kapilarini sunar.

HNLang'in uzun vadeli rolu:

- Sistem gorevlerini betiklemek.
- HNShell komut modelini genisletmek.
- Knowledge FS nesnelerini ifade etmek.
- AI tarafindan onerilen planlari denetlenebilir sistem islemlerine cevirmek.

## AI'nin Kernel Icine Gomulmemesi

AI dogrudan kernel icine gomulmemelidir.

Bunun nedenleri:

- Kernel deterministik, kucuk ve denetlenebilir kalmalidir.
- AI davranisi yuksek seviyeli, degisken ve izin gerektiren bir sistem davranisidir.
- Kernel icindeki AI, hata ayiklama ve guvenlik sinirlarini belirsizlestirir.
- Model, prompt, baglam ve gorev politikasi kernel ABI'sinin parcasi olmamalidir.

HNMos'ta AI, kernel'in kendisi degil, OS tarafindan denetlenen bir sistem arayuzuyle calisan yardimci zeka katmanidir.

## AI Icin Sistem Arayuzu Tasarimi

AI-Native System Interface Layer su soyutlamalari tasir:

- Event Stream: kernel ve Core OS olaylarinin AI tarafindan okunabilir ozetleri.
- Task Graph: kullanici niyetlerinin parcalanmis ve izlenebilir gorev adimlari.
- Permission Gate: AI'nin hangi sistem eylemlerini onerebilecegi veya calistirabilecegi.
- Context Broker: dosya, shell, gorev ve cihaz baglaminin sinirli bicimde sunulmasi.
- Audit Log: AI etkilesimlerinin ve sistem eylemlerinin kaydedilmesi.

AI, bu arayuzler uzerinden sistemle etkilesir. Kernel sadece yetki, olay ve syscall sinirlarini uygular.

## Uzun Vadeli Bare-Metal Hedef

Uzun vadede HNMos:

- QEMU disinda gercek x86 donanimda boot edebilmelidir.
- Kendi boot yolunu GRUB bagimliligindan ayirabilmelidir.
- Bellek, interrupt, scheduler ve dosya sistemi altyapisini kendi icinde tasimalidir.
- HNShell ve HNLang'i user-space sistem araclari olarak calistirmalidir.
- AI-native arayuzu model bagimsiz, izinli ve denetlenebilir bir OS servisi olarak sunmalidir.

## HNMOS-CODEX-01 Tamamlanma Kriterleri

Bu kademe su kosullarda tamamlanmis sayilir:

- HNMos'un host prototype olmayacagi mimari belgelerde acikca belirtilir.
- Toolchain ve hedef OS ayrimi netlestirilir.
- Kernel/user-space ayrimi tanimlanir.
- Kernel 01 hedefi ve sinirlari yazilir.
- Core OS bilesenleri listelenir.
- HNShell ve HNLang'in OS icindeki yeri tanimlanir.
- AI'nin kernel icine gomulmemesi karari gerekcelendirilir.
- AI icin olay, gorev, izin, baglam ve audit temelli sistem arayuzu tasarlanir.

## HNMOS-CODEX-02'ye Gecis Plani

HNMOS-CODEX-02, bu mimari kararlarin ilk teknik uygulama kademesi olmalidir.

CODEX-02 icin onerilen kapsam:

1. Kernel 01 build zincirini kesinlestir.
2. QEMU boot testini arac eksikleri giderildikten sonra calistir.
3. GDT ve IDT icin `kernel01/interrupts/` altinda ilk dosya yapisini kur.
4. Exception handler ve basit panic ekrani ekle.
5. Port I/O yardimcilarini hazirla.
6. Timer kesmesine hazir olacak cekirdek log modelini baslat.

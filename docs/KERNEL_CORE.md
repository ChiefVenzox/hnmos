# Kernel Core

## Hedef

HNMOS-CODEX-04'un hedefi, Kernel 01 icin temel isletim sistemi altyapilarini kademe kademe tasarlamaktir.

Bu asamada AI eklenmez. Kernel once bellek, interrupt, timer, scheduler ve syscall sinirlariyla saglamlasmalidir. AI-native katman daha sonra bu sinirlarin uzerinde, izinli OS arayuzleriyle calisacaktir.

## Dosya Yapisi

```text
kernel01/
|-- memory/
|-- interrupts/
|-- timer/
|-- scheduler/
`-- syscalls/

docs/
|-- KERNEL_CORE.md
|-- MEMORY.md
|-- INTERRUPTS.md
`-- SYSCALLS.md
```

## CODEX-04 Alt Kademeleri

### HNMOS-CODEX-04.1 Memory Map

Amac, bootloader tarafindan verilen bellek bilgisini okuyup kernel icin guvenilir bir fiziksel bellek haritasi olusturmaktir.

Uygulanabilir sira:

1. Multiboot info yapisinin minimum alanlarini tanimla.
2. `mmap_addr` ve `mmap_length` alanlarini dogrula.
3. Kullanilabilir ve rezerve bellek araliklarini serial log'a yaz.
4. Kernel imajinin kapladigi alanlari ayrica rezerve kabul et.
5. Ilk fiziksel sayfa bitmap planini hazirla.

### HNMOS-CODEX-04.2 Paging

Amac, Kernel 01'i kimlik eslemeli basit paging ile calistirmaya hazirlamaktir.

Uygulanabilir sira:

1. 4 KiB page ve page directory/table sabitlerini tanimla.
2. Ilk 4 MiB veya 16 MiB alani identity map yap.
3. VGA bellegini ve kernel imaj araligini map icinde tut.
4. CR3 yukleme ve CR0 paging bitini acma kodunu ayir.
5. Page fault handler CODEX-04.3 ile baglanacak sekilde planla.

### HNMOS-CODEX-04.3 Interrupts

Amac, CPU exception ve IRQ olaylarini kernel tarafindan yakalanabilir hale getirmektir.

Uygulanabilir sira:

1. GDT descriptor yapilarini tanimla.
2. IDT gate yapilarini tanimla.
3. Assembly interrupt stub'larini ekle.
4. Divide-by-zero exception testini panic sistemine bagla.
5. PIC remap ve IRQ maskeleme planini hazirla.

### HNMOS-CODEX-04.4 Timer

Amac, donanim zamanlayici kesmesini kernel tick kaynagina cevirmektir.

Uygulanabilir sira:

1. PIT 8253/8254 port sabitlerini tanimla.
2. Timer frekansi sec.
3. IRQ0 handler'i tick sayacini artiracak hale getir.
4. Serial log'a ilk tick bilgisi icin kontrollu debug yolu ekle.
5. Scheduler'a tick bildirim arayuzu tanimla.

### HNMOS-CODEX-04.5 Basic Scheduler

Amac, hemen tam user-space calistirmadan once basit kernel task modelini tasarlamaktir.

Uygulanabilir sira:

1. Task state enum'u tanimla.
2. Kernel task control block taslagi olustur.
3. Round-robin secim mantigini dokumante et.
4. Context switch icin kaydedilecek register setini belirle.
5. Timer tick ile scheduler arasindaki cagrilma sinirini tanimla.

### HNMOS-CODEX-04.6 Syscall Interface

Amac, uzun vadeli user-space servis kapisini tasarlamaktir.

Uygulanabilir sira:

1. Syscall numaralandirma politikasini belirle.
2. Register tabanli syscall ABI taslagi yaz.
3. Ilk syscall adaylarini sec: write, exit, yield, get_time.
4. Kernel tarafinda syscall dispatcher planini hazirla.
5. User-space henuz yokken syscall testleri icin kernel ic test stublari planla.

## Kernel Tarafinda AI'ye Dogrudan Izin Verilmemesi

CODEX-04, AI ekleme kademesi degildir.

Kernel su ilkeyi korur:

- AI modeli kernel icinde calismaz.
- AI kernel fonksiyonlarini dogrudan cagiramaz.
- AI bellek, interrupt, scheduler veya syscall politikalarini atlayamaz.
- AI icin ileride acilacak tum kapilar syscall, izin ve audit katmanlari uzerinden tasarlanir.

Bu ilke, kernel'in deterministik ve denetlenebilir kalmasi icin zorunludur.

## Tamamlanma Kriterleri

HNMOS-CODEX-04 tasarim kademesi su kosullarda tamamlanir:

- `kernel01/memory/`, `kernel01/interrupts/`, `kernel01/timer/`, `kernel01/scheduler/`, `kernel01/syscalls/` dizinleri hazirdir.
- Memory map ve paging icin gelistirme sirasi tanimlanmistir.
- Interrupt, timer ve scheduler iliskisi tanimlanmistir.
- Syscall arayuzunun ilk ABI plani yazilmistir.
- Kernel tarafinda AI'ye dogrudan izin verilmeyecegi ilke olarak kayda gecmistir.

## HNMOS-CODEX-05'e Gecis

CODEX-05, bu planin ilk uygulama kademesi olmalidir:

1. Multiboot memory map yapilarini C tarafinda tanimla.
2. Kullanilabilir bellek araliklarini serial log'a yaz.
3. Kernel imaj araligini rezerve et.
4. Ilk fiziksel sayfa ayirici taslagini ekle.
5. Paging uygulamasina gecmeden once memory map ciktisini QEMU'da dogrula.

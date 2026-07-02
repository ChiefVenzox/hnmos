# Interrupts, Timer, Scheduler

## Hedef

Bu belge, CODEX-04.3 Interrupts, CODEX-04.4 Timer ve CODEX-04.5 Basic Scheduler planini tanimlar.

## Dosya Yapisi

```text
kernel01/
|-- interrupts/
|   |-- gdt.c
|   |-- gdt.h
|   |-- idt.c
|   |-- idt.h
|   |-- isr.s
|   |-- isr.c
|   `-- isr.h
|-- timer/
|   |-- pit.c
|   |-- pit.h
|   |-- timer.c
|   `-- timer.h
`-- scheduler/
    |-- scheduler.c
    |-- scheduler.h
    |-- task.c
    `-- task.h
```

Bu dosyalar uygulanacak hedef yapidir. CODEX-04 bu yapinin siralamasini belirler.

## Interrupt Sistemi

Interrupt sistemi kernel'in CPU olaylarini yakalamasini saglar.

Ilk kapsam:

- GDT kurulumu.
- IDT kurulumu.
- CPU exception handler'lari.
- PIC remap.
- IRQ handler kaydi.
- Panic sistemiyle exception baglantisi.

## GDT Plani

GDT erken evrede uc temel descriptor tasimalidir:

- Null descriptor.
- Kernel code segment.
- Kernel data segment.

Ilk hedef user-space degildir. User-space descriptor'lari syscall ve scheduler altyapisi olgunlastiktan sonra eklenmelidir.

Gelistirme sirasi:

1. GDT entry ve pointer yapilarini tanimla.
2. Kernel code/data descriptor'larini doldur.
3. `lgdt` assembly yardimcisini yaz.
4. Segment register'larini yenile.
5. Serial log ile GDT kurulumunu dogrula.

## IDT Plani

IDT, exception ve IRQ kapilarini tanimlar.

Ilk hedef exception 0-31 araligidir:

- Divide-by-zero.
- Invalid opcode.
- General protection fault.
- Page fault.

Gelistirme sirasi:

1. IDT gate yapisini tanimla.
2. IDT pointer yapisini tanimla.
3. Assembly ISR stub'larini ekle.
4. C tarafinda ortak `hnm_isr_dispatch` fonksiyonu yaz.
5. Beklenmeyen exception durumunda `hnm_panic` cagir.

## PIC ve IRQ Plani

QEMU i386 hedefinde ilk IRQ yonetimi icin 8259 PIC kullanilabilir.

Plan:

- Master/slave PIC remap yap.
- IRQ0 timer icin acik birak.
- Diger IRQ'lari ilk evrede maskele.
- End-of-interrupt sinyalini dogru PIC'e gonder.

## Timer Sistemi

Timer, scheduler icin ilk zaman kaynagidir.

Ilk hedef PIT olmalidir:

- PIT channel 0.
- IRQ0.
- Basit global tick sayaci.
- Frekans: 100 Hz veya 1000 Hz karari sonraki uygulamada verilir.

## Mevcut Minimal Input IRQ Uygulamasi

HNMOS-CODEX-04 ve HNMOS-CODEX-05 ile planin dar bir parcasi uygulanmistir:

- `kernel01/interrupts/interrupts.*`: IDT, PIC remap ve IRQ1/IRQ12 unmask.
- `kernel01/interrupts/isr.s`: IRQ1 keyboard ve IRQ12 mouse assembly stub'lari.
- `kernel01/drivers/keyboard.*`: PS/2 scancode okuma ve UI event push.
- `kernel01/drivers/mouse.*`: PS/2 mouse paket okuma ve cursor state guncelleme.
- `kernel01/kernel/interface/events.*`: keyboard ve mouse icin ortak sabit boyutlu UI event queue.
- `kernel01/kernel/interface/input_dispatcher.*`: ana dongude event poll ve ilk terminal dispatch yolu.
- `kernel01/console/terminal.*`: keyboard event poll ve grafik terminal.
- `kernel01/kernel/graphics/cursor.*`: framebuffer cursor save/restore ve status cizimi.

Bu uygulama sadece erken keyboard/mouse input icindir. CPU exception stublari, IRQ0 timer ve genel IRQ dispatcher hala TODO'dur.

Gelistirme sirasi:

1. PIT port sabitlerini tanimla.
2. Divisor hesaplama fonksiyonu ekle.
3. IRQ0 handler icinde tick sayacini artir.
4. Belirli araliklarla serial debug log bas.
5. Scheduler tick hook'unu cagir.

## Basic Scheduler Mantigi

CODEX-04 scheduler hedefi tam process sistemi degildir. Ilk hedef, kernel icinde task kavramini ve timer tick ile cagrilabilecek secim mantigini tasarlamaktir.

Ilk task modeli:

```text
task_id
state
kernel_stack
saved_registers
next
```

Task state:

- Ready.
- Running.
- Blocked.
- Dead.

Ilk scheduler algoritmasi round-robin olmalidir. Priority, sleep queue ve user process destegi sonraki kademelere birakilir.

## HNMOS-CODEX-04.3 Interrupts Gelistirme Sirasi

1. `kernel01/interrupts/gdt.*` dosyalarini uygula.
2. `kernel01/interrupts/idt.*` dosyalarini uygula.
3. `kernel01/interrupts/isr.s` stub'larini ekle.
4. Divide-by-zero exception testini ekle.
5. Page fault icin panic mesaj formatini hazirla.

## HNMOS-CODEX-04.4 Timer Gelistirme Sirasi

1. PIC remap olmadan timer'a gecme.
2. PIT kurulumu yap.
3. IRQ0 handler ile tick sayacini artir.
4. Tick sayacini serial log ile dogrula.
5. Scheduler hook'u icin bos fonksiyon ekle.

## HNMOS-CODEX-04.5 Basic Scheduler Gelistirme Sirasi

1. Task veri yapisini tanimla.
2. Tek kernel task'i `running` olarak kaydet.
3. Round-robin liste yapisini hazirla.
4. Timer tick uzerinden scheduler cagrisi icin hook ekle.
5. Context switch assembly kodunu sonraki uygulama kademesine birak.

## AI Ilkesi

Interrupt, timer ve scheduler kernel guvenlik siniridir.

AI:

- Interrupt handler calistiramaz.
- IRQ maskeleme politikasini degistiremez.
- Scheduler kararlarini dogrudan belirleyemez.
- Kernel task context'lerine erisemez.

AI ileride sadece OS tarafindan uretilen denetlenmis olay ozetlerini okuyabilir.

## Tamamlanma Kriteri

- GDT/IDT/ISR uygulama sirasi tanimlanmistir.
- PIC ve PIT plani yazilmistir.
- Timer tick ile scheduler arasindaki iliski belirlenmistir.
- Basic scheduler icin task modeli ve round-robin yaklasimi tanimlanmistir.

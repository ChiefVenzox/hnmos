# Memory

## Hedef

Bu belge, Kernel 01 icin bellek yonetimi planini tanimlar. CODEX-04.1 Memory Map ve CODEX-04.2 Paging bu belgenin kapsamindadir.

## Dosya Yapisi

```text
kernel01/kernel/
`-- memory/
    |-- pmm.c
    |-- pmm.h
    |-- heap.c
    `-- heap.h
```

HNMOS-CODEX-09 ile PMM ve erken heap bu dizin altinda uygulanmistir. Paging henuz uygulanmamistir.

## Mevcut Uygulama

Kernel 01 su anda Multiboot memory map bilgisini okur, fiziksel sayfalari bitmap ile takip eder ve PMM uzerinde kucuk bir kernel heap baslatir.

Acilis serial log ornegi:

```text
memory: PMM ready.
memory total kib:    0x0001FFC0
memory usable kib:   0x0001FDFF
memory reserved kib: 0x000001C1
memory free pages:   0x00007EC9
memory mmap entries: 0x00000006
heap: bump allocator ready.
heap: smoke allocation ok.
```

QEMU launcher icindeki Memory screen ayni bilgileri grafik olarak gosterir.

## Bellek Yonetimi Plani

Kernel 01 bellek yonetimi uc asamada gelismelidir:

1. Memory map okuma.
2. Fiziksel sayfa yonetimi.
3. Paging ve sanal adres alani.

Ilk hedef heap veya user-space memory degil, kernel'in hangi fiziksel bellek araliklarini kullanabilecegini bilmesidir.

## Memory Map Okuma Mantigi

Kernel 01 Multiboot ile acildigi icin bootloader `multiboot_info` adresini kernel'e verir. `hnm_kernel_main`, bu adresi su amacla kullanmalidir:

- Multiboot magic degerini dogrulamak.
- Memory map flag'inin varligini kontrol etmek.
- `mmap_addr` ve `mmap_length` alanlarini okumak.
- Her memory map entry'sini gezmek.
- Kullanilabilir bellek ile rezerve bellek araliklarini ayirmak.

Ilk log formati:

```text
memory: available base=0x00000000 length=0x0009FC00
memory: reserved  base=0x0009FC00 length=0x00000400
```

## Rezerve Edilecek Alanlar

Memory map ne derse desin kernel su alanlari kullanilabilir saymamalidir:

- Kernel ELF'in yuklendigi aralik.
- Kernel stack.
- VGA memory: `0xB8000`.
- Multiboot info ve memory map yapilarinin bulundugu alan.
- BIOS/firmware tarafindan rezerve edilen dusuk bellek bolgeleri.

`linker.ld` icindeki `kernel_end` sembolu, kernel imajinin bitis adresini hesaplamak icin kullanilabilir.

## Fiziksel Sayfa Yoneticisi

Ilk fiziksel memory manager basittir:

- Page size: 4096 byte.
- Her fiziksel sayfa icin bitmap biti.
- Kullanilabilir memory map araliklari once free isaretlenir.
- Kernel ve rezerve araliklar tekrar used isaretlenir.
- Dusuk 1 MiB bellek rezerve edilir.
- Kernel image ve statik bitmap `kernel_end` sembolune kadar rezerve edilir.
- 32-bit erken evre icin en fazla 1 GiB fiziksel bellek takip edilir.

API:

```text
hnm_pmm_init(multiboot_info_addr)
hnm_pmm_alloc_page()
hnm_pmm_free_page(page)
hnm_pmm_get_stats(out_stats)
hnm_pmm_log_stats()
```

`hnm_pmm_free_page()` su an ayrilmis sayfayi bitmapte tekrar free isaretler. Ayrica owner tracking veya double-free korumasi yoktur; bu sonraki bellek guclendirme adimidir.

## Kernel Heap

Heap `kernel01/kernel/memory/heap.*` icindedir.

Mevcut heap, PMM'den sayfa alip bu sayfalar uzerinden 8 byte hizali bump allocation yapar:

```text
hnm_heap_init()
hnm_kmalloc(size)
hnm_kfree(ptr)
hnm_heap_get_stats(out_stats)
```

Sinirlar:

- `hnm_kfree()` su an no-op'tur.
- Allocation en fazla bir sayfa boyutundadir.
- Free list, coalescing ve realloc yoktur.
- Heap paging veya virtual memory kullanmaz; Kernel 01 henuz identity physical addressing varsayimi ile calisir.

Bu tasarim UI, memory screen ve ilerideki kernel-side metadata icin kontrollu bir baslangic saglar; genel amacli heap degildir.

## Paging Plani

Paging erken evrede basit identity map ile baslamalidir.

Ilk hedef:

- Kernel'in calistigi fiziksel adresler ayni sanal adreslere map edilir.
- VGA bellegi map edilir.
- Ilk 4 MiB veya 16 MiB identity map yapilir.
- Page fault exception CODEX-04.3 interrupt sistemiyle baglanir.

Paging acma sirasi:

1. Page directory ve page table alanlarini hizali ayir.
2. Identity map entry'lerini doldur.
3. CR3 register'ina page directory adresini yaz.
4. CR0 icindeki paging bitini ac.
5. Paging acildiktan sonra serial log ile devam edildigini dogrula.

## HNMOS-CODEX-04.1 Memory Map Gelistirme Sirasi

1. Multiboot bilgi yapilarini `memory_map.h` icinde tanimla.
2. `hnm_memory_map_init(multiboot_info_addr)` fonksiyonunu ekle.
3. Memory map flag yoksa `mem_lower`/`mem_upper` fallback yolunu kullan.
4. Entry ve toplam istatistikleri serial log'a yaz.
5. Kullanilabilir toplam bellek miktarini hesapla.

## HNMOS-CODEX-04.2 Paging Gelistirme Sirasi

1. Paging sabitlerini `paging.h` icinde tanimla.
2. Hizali page directory/page table alanlari ayir.
3. Identity map fonksiyonunu yaz.
4. Paging acma assembly yardimcisini ekle.
5. Page fault handler icin interrupt kademesine baglanti noktasi birak.

## AI Ilkesi

Bellek yonetimi kernel icindedir ve AI tarafindan dogrudan kontrol edilemez.

AI ileride bellekle ilgili yalnizca denetlenmis sistem metriklerini okuyabilir. Fiziksel sayfa ayirma, page table degistirme veya kernel adreslerine erisim gibi islemler AI'ye acik dogrudan kapilar olmayacaktir.

## Tamamlanma Kriteri

- Memory map okuma stratejisi tanimlanmistir.
- Kernel tarafindan rezerve edilecek araliklar listelenmistir.
- Fiziksel sayfa yoneticisi icin ilk API plani yazilmistir.
- Paging icin identity map ve CR0/CR3 acma sirasi belirlenmistir.

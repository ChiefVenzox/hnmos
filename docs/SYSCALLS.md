# Syscalls

## Hedef

Bu belge, CODEX-04.6 Syscall Interface planini tanimlar.

Syscall arayuzu, user-space kodun kernel servislerine kontrollu erisim kapisidir. Kernel icinde AI'ye dogrudan izin verilmez; ileride AI-native servisler de ancak bu tip denetlenmis OS kapilarinin uzerinden hareket edebilir.

## Dosya Yapisi

```text
kernel01/
`-- syscalls/
    |-- syscall.c
    |-- syscall.h
    |-- syscall_numbers.h
    |-- syscall_dispatch.c
    `-- syscall_dispatch.h
```

Bu dosyalar CODEX-04.6 icin hedef yapidir.

## Syscall Arayuzu

Syscall arayuzu su sorumluluklara sahiptir:

- User-space istegini kernel tarafinda dogrulamak.
- Syscall numarasini izinli handler'a yonlendirmek.
- Argumanlari ABI'ye gore okumak.
- Gecersiz istekleri guvenli hata koduyla dondurmek.
- Kernel pointer'larini user-space'e dogrudan acmamak.

## Ilk Syscall ABI Plani

Erken i386 ABI taslagi:

```text
eax = syscall number
ebx = arg0
ecx = arg1
edx = arg2
esi = arg3
edi = arg4
return = eax
```

Syscall kapisi icin ilk secenek `int 0x80` olabilir. Daha sonra `sysenter` veya mimariye ozel daha hizli yollar degerlendirilebilir.

## Ilk Syscall Adaylari

Ilk syscall listesi kucuk tutulmalidir:

```text
0: hnm_sys_write
1: hnm_sys_exit
2: hnm_sys_yield
3: hnm_sys_get_time
```

Ilk uygulama sirasinda user-space henuz olmadigi icin bu syscall'lar kernel ic test stub'lariyla dogrulanabilir.

## Syscall Dispatcher Mantigi

Dispatcher su akisla calismalidir:

```text
interrupt/syscall entry
  -> register frame
  -> syscall number validation
  -> handler lookup
  -> argument validation
  -> handler execution
  -> return value in eax
  -> iret
```

Gecersiz syscall numarasi:

```text
return -1
```

Kernel panic yalnizca kernel ic tutarsizliklarinda kullanilmalidir; kullanici tarafindan gelen gecersiz syscall normal hata olarak donmelidir.

## Guvenlik Sinirlari

Syscall sistemi, kernel/user-space ayriminin ilk gercek kapisidir.

Kurallar:

- User pointer'lari dogrulanmadan okunmaz.
- Kernel adresleri user-space'e dondurulmez.
- Syscall handler'lari interrupt state ve scheduler state ile uyumlu olmalidir.
- Kalici veya tehlikeli islemler icin ileride permission layer gerekir.

## Kernel Tarafinda AI'ye Dogrudan Izin Verilmemesi

AI syscall arayuzunu atlayamaz.

Ilke:

- AI kernel memory'ye dogrudan erisemez.
- AI syscall dispatcher icinde ozel ayricalik almaz.
- AI sadece ileride user-space veya sistem servisi olarak calisir.
- AI kaynakli eylemler permission ve audit katmanlarindan gecmeden kernel servislerini calistiramaz.

CODEX-04'te AI kodu yazilmayacaktir.

## HNMOS-CODEX-04.6 Gelistirme Sirasi

1. Syscall numaralarini `syscall_numbers.h` icinde tanimla.
2. Register frame yapisini interrupt sistemiyle ortak hale getir.
3. `int 0x80` IDT gate'ini planla.
4. Dispatcher tablosunu yaz.
5. Ilk `write`, `yield`, `get_time` stub'larini ekle.
6. User-space henuz yokken kernel ic test cagrilariyla dispatcher'i dogrula.

## HNMOS-CODEX-05'e Hazirlik

Syscall uygulamasina gecmeden once CODEX-05 memory map uygulamasi tamamlanmalidir. User pointer dogrulama, paging ve memory ownership olmadan guvenli syscall sistemi eksik kalir.

## Tamamlanma Kriteri

- Syscall ABI taslagi tanimlanmistir.
- Ilk syscall adaylari listelenmistir.
- Dispatcher akisi yazilmistir.
- AI'nin syscall sisteminde ozel kernel ayricaligi olmayacagi belirtilmistir.

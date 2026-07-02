# HNMOS-CODEX

## Proje Amaci

HNMos, AI destekli, gorev odakli, sifirdan gelistirilen deneysel bir isletim sistemidir.

Ilham kaynagi TempleOS'un sade, kisisel ve bilgisayar ile dogrudan konusan isletim sistemi felsefesidir. HNMos, TempleOS kopyasi degildir; kendi cekirdek, kabuk, dil, bilgi dosya sistemi ve AI izin modelini asamali olarak insa eder.

## Ana Karar

HNMos normal Linux/macOS/Windows uzerinde calisan bir prototip sistem olmayacaktir.

HNMos sifirdan yazilacaktir. Ilk hedef, QEMU uzerinde boot eden gercek bare-metal `Kernel 01` cekirdegidir. Linux/macOS/Windows sadece gelistirme, derleme ve QEMU calistirma ortami olarak kullanilir.

## Ana Parcalar

- Kernel 01
- HNShell
- AI Runtime Interface
- HNLang
- Knowledge FS
- Driver Event Layer
- AI Permission Layer
- HNMos SDK

## Calisma Kurali

Her kademe su sirayla ele alinir:

1. Hedef aciklanir.
2. Dosya yapisi verilir.
3. Uygulanabilir belge, kod veya plan uretilir.
4. Her sey sifirdan isletim sistemi gelistirme mantigiyla kurgulanir.
5. En sonda tamamlanma kriteri ve sonraki kademe yazilir.

## Kademe Mantigi

Her kademe, host OS bagimliligini azaltan veya HNMos'un kendi sistem soyutlamasini artiran somut bir cikti uretmelidir. Uygulama prototipleri, web arayuzleri veya masaustu simulasyonlari HNMos'un ana yolu degildir.

## Aktif Kademe

`HNMOS-CODEX-14`: HNMos v0.0.1 ilk demo surumu. Demo host uygulamasi degil, QEMU uzerinde boot eden Kernel 01 baslangicidir; build, run, release notes, demo dokumani ve roadmap hazirlanir.

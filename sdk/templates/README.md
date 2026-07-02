# HNMos SDK Templates

Bu klasor, HNMos uzerinde calisacak sistem araclari, gorev uygulamalari ve AI destekli yardimci uygulamalar icin sablonlar tutar.

Bu sablonlar Windows/Linux/macOS uygulamasi uretmek icin degildir. Host sistem yalnizca HNMos imaji, toolchain ve QEMU calistirma ortamidir.

Ilk sablonlar:

- `app.manifest.json`: uygulama kimligi, entry point, izinler ve HNShell calistirma bilgisi.
- HNLang task app sablonu: ileride `main.hnl` ile eklenecek.
- Native user-space app sablonu: ileride freestanding C/HNLang ABI ile eklenecek.

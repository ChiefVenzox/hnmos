# HNMos Test Tools

Bu klasor, HNMos'un bare-metal gelistirme akisini test etmek icin host tarafinda calisan yardimci scriptleri tutar.

Host scriptleri HNMos uygulamasi degildir. Yalnizca kernel ELF dogrulama, QEMU smoke test, manifest kontrolu ve CI otomasyonu icin kullanilir.

Ilk araclar:

- `static-check.sh`: kernel ELF/multiboot ve manifest JSON dogrulamasi.
- `qemu-smoke.sh`: QEMU uzerinde Kernel 01 acilis mesajlarini serial log'dan kontrol eden smoke test.

# HNMos FS

## HNMOS-CODEX-10 Calisan Durum

Kernel 01 artik ilk calisan filesystem foundation katmanina sahiptir:

```text
kernel01/kernel/fs/vfs.*
kernel01/kernel/fs/ramfs.*
```

Bu uygulama long-term metadata/Knowledge FS hedeflerinin yerine gecmez. Simdilik en guvenli boot yolu olarak kernel icine gomulu read-only RAM node'lari sunar:

```text
/system/version
/system/info
/readme.txt
```

Grafik terminalde `ls` ve `cat` komutlari bu node'lari VFS uzerinden okur. Ayrintilar icin `docs/FILESYSTEM.md` dosyasina bak.

## Hedef

HNMOS-CODEX-08'in kernel tarafi hedefi, HNMos icin ilk minimal dosya ve metadata modelini tasarlamaktir.

Bu dosya sistemi Windows/Linux/macOS uzerinde calisan ayri bir prototip olmayacaktir. HNMos'un kendi file system ve metadata mantigi icinde, bare-metal OS hedefiyle tasarlanacaktir.

## Dosya Yapisi

```text
kernel01/
`-- fs/
    |-- metadata/
    |   |-- metadata.c
    |   |-- metadata.h
    |   |-- tag.c
    |   `-- tag.h
    |-- fs_node.c
    |-- fs_node.h
    |-- vfs.c
    `-- vfs.h
```

Bu dosyalar CODEX-08 tasarim hedefidir. Bu kademede klasorler hazirlanir ve ilk model tanimlanir.

## HNMos File System Hedefi

HNMos FS uzun vadede sadece byte saklayan bir dosya sistemi olmayacaktir.

Hedef:

- Dosya ve dizinleri temsil etmek.
- Metadata tasimak.
- Ozet, etiket ve index bilgisiyle Knowledge FS'e veri saglamak.
- Kernel/user-space sinirlarini korumak.
- AI Runtime Interface'e sadece izinli metadata ve ozet sunmak.

## Ilk Minimal Dosya Modeli

Ilk dosya modeli basit tutulmalidir:

```text
fs_node
  id
  type
  path
  size
  flags
  metadata_id
```

Node tipleri:

- File.
- Directory.
- Device.
- Virtual.

Ilk asamada gercek disk surucusu zorunlu degildir. Initramfs, memory-backed FS veya sabit kernel test node'lari ile model dogrulanabilir.

## Metadata Yapisi

Metadata dosya iceriginin kendisi degildir. Dosya hakkinda guvenli ve sinirli bilgi tasir.

Ilk metadata alanlari:

```text
metadata_id
node_id
created_tick
modified_tick
owner
content_type
summary_id
tag_count
index_state
permission_flags
```

Metadata kernel tarafinda guvenli okunur. AI tarafina ham kernel yapilari, pointer'lar veya fiziksel adresler verilmez.

## Kernel Seviyesinde Guvenli Metadata Okuma

Kernel metadata okuma kurallari:

- Metadata pointer'i user-space'e dogrudan verilmez.
- Path veya node id dogrulanmadan metadata okunmaz.
- Permission flag kontrolu olmadan ozet veya etiket sunulmaz.
- Kernel-only metadata alanlari user-space veya AI tarafina acilmaz.
- Metadata snapshot kopyasi uzerinden okunur.

Ilk guvenli API taslagi:

```text
hnm_fs_get_metadata(node_id, out_snapshot)
hnm_fs_get_summary(metadata_id, out_summary)
hnm_fs_list_tags(metadata_id, out_tags)
```

## Metadata ve Syscall Iliskisi

User-space HNShell, HNLang veya AI Runtime Interface metadata'ya dogrudan kernel belleginden erisemez.

Dogru akis:

```text
user-space request
  -> syscall / Core OS FS service
  -> permission check
  -> metadata snapshot
  -> caller
```

## HNShell ile Kullanim

Ilk HNShell komutlari:

```text
hn fs list
hn fs meta /system/config
hn memory search "scheduler"
hn summarize docs
hn index kernel
```

Ilk cevaplar gercek index hazir degilken plan/stub seklinde olabilir:

```text
fs: metadata layer planned
index: not built
```

## Tamamlanma Kriterleri

- `kernel01/fs/metadata/` klasoru hazirdir.
- Ilk dosya node modeli tanimlanmistir.
- Metadata alanlari belirlenmistir.
- Kernel seviyesinde guvenli metadata okuma kurallari yazilmistir.
- HNShell komutlari icin FS tarafindaki hedefler belirtilmistir.

## HNMOS-CODEX-08 Durumu

CODEX-08, ilk minimal FS metadata uygulamasi icin su hedefleri tasarim seviyesinde belirlemistir:

- `fs_node` ve `metadata` struct taslaklari.
- Sabit `/system/config` test node'u.
- `hn fs list` ve `hn fs meta /system/config` icin stub veri.
- Metadata snapshot okuma fonksiyonu.
- AI index henuz yokken `index_state = not_built` cevabi.

# Knowledge FS

## Hedef

Knowledge FS'in hedefi, HNMos dosya sistemini sadece dosya saklayan bir yapi olmaktan cikarip AI icin anlamlandirilabilir, aranabilir ve ozetlenebilir hale getirmektir.

Bu sistem Windows/Linux/macOS uzerinde calisan ayri bir prototip indeksleyici olmayacaktir. HNMos'un kendi file system, metadata, event ve AI Runtime Interface katmanlari icinde tasarlanacaktir.

## Dosya Yapisi

```text
kernel01/
`-- fs/
    `-- metadata/

ai/
|-- memory_index/
|-- semantic_fs/
`-- indexer/

docs/
|-- HNMOS_FS.md
`-- KNOWLEDGE_FS.md
```

## Knowledge FS Amaci

Knowledge FS su soruya cevap verir:

```text
Bu dosya sistemindeki bilgi AI ve kullanici icin nasil anlamli hale gelir?
```

Temel akis:

```text
Dosya
  -> Metadata
  -> Ozet
  -> Etiket
  -> Indeks
  -> AI sorgu arayuzu
```

## Metadata Yapisi

Metadata, Knowledge FS'in cekirdek baglamidir.

Ilk metadata alanlari:

```text
path
node_type
content_type
size
summary_ref
tags
index_state
last_indexed_tick
permissions
```

Metadata kernel tarafinda uretilir veya dogrulanir. AI Runtime Interface metadata'nin kopyalanmis ve izinli ozetini gorur.

## Ozetleme Mantigi

Ozet, dosya iceriginin kisa ve guvenli temsilidir.

Ilk asamada gercek AI ozetleme zorunlu degildir. Ozet kaynaklari:

- Elle tanimlanmis sabit ozet.
- Dosya turune gore basit sistem ozeti.
- HNLang veya HNShell tarafindan uretilen task sonucu.
- Gelecekte AI Runtime Interface tarafindan onerilen ozet.

Ozet yazma kurali:

- AI ozet onerisi dogrudan metadata'ya yazilmaz.
- Once Safety Layer ve permission kontrolu gerekir.
- Audit/event kaydi uretilir.

## Etiketleme Sistemi

Etiketler dosyanin anlamli siniflandirmasidir.

Ilk etiket tipleri:

```text
system
kernel
config
driver
doc
task
ai-generated
needs-review
```

Etiket kaynaklari:

- Kernel/Core OS.
- HNShell komutu.
- HNLang task script.
- AI Runtime Interface onerisi.

AI tarafindan onerilen etiketler varsayilan olarak `needs-review` ile isaretlenmelidir.

## Indeks Plani

Index, arama ve AI sorgu arayuzu icin hazirlanan veri yapisidir.

Ilk index hedefleri:

- Path index.
- Tag index.
- Summary text index.
- Content type index.
- Task/event reference index.

Ilk index state degerleri:

```text
not_built
pending
indexed
stale
denied
```

Indexer, kernel icinde AI modeli calistirmaz. Kernel sadece metadata ve event kaydi saglar. Indexer uzun vadede user-space servis olarak calismalidir.

## AI Sorgu Arayuzu

AI Runtime Interface Knowledge FS'e dogrudan raw FS erisimiyle ulasmaz.

Dogru akis:

```text
AI request
  -> Context Router
  -> Knowledge FS query policy
  -> metadata/summary/index snapshot
  -> AI Runtime Interface
```

AI'nin gorebilecegi veri:

- Izinli metadata snapshot.
- Izinli ozet.
- Izinli etiketler.
- Index sonucu.

AI'nin goremeyecegi veri:

- Kernel pointer.
- Ham disk bloklari.
- Fiziksel bellek.
- Permission ile kapali dosya icerigi.
- Gizli sistem token'lari.

## HNShell ile Kullanim

Ilk komutlar:

```text
hn fs list
hn fs meta /system/config
hn memory search "scheduler"
hn summarize docs
hn index kernel
```

Komut anlamlari:

- `hn fs list`: FS node listesini gosterir.
- `hn fs meta /system/config`: dosya metadata snapshot'ini gosterir.
- `hn memory search "scheduler"`: Knowledge FS index uzerinde semantic arama hedefidir.
- `hn summarize docs`: docs alaninin ozetlenmesini task olarak ister.
- `hn index kernel`: kernel alanini indexleme task'i olusturur.

Ilk asamada komutlar stub veya plan cevabi dondurebilir; host FS taramasi yapmaz.

## AI Runtime Interface ile Baglanti

Knowledge FS, AI Runtime Interface icin baglam kaynaklarindan biridir.

Baglanti kurallari:

- Context Router izinli Knowledge FS snapshot'i hazirlar.
- Safety Layer sorgu riskini siniflandirir.
- Task Engine index veya summarize islerini task olarak izler.
- AI Runtime cevabi ozet/etiket onerisi ise dogrudan yazilmaz.
- Metadata degisikligi permission ve audit gerektirir.

## Kernel Seviyesinde Guvenli Metadata Okuma

Kernel metadata okuma su sinirlari korumalidir:

- Snapshot kopyasi kullan.
- Permission flag kontrol et.
- Path normalize et.
- Node type dogrula.
- Kernel-only alanlari maskele.
- AI veya user-space tarafina pointer verme.

## Ilk Minimal Dosya/Metadata Tasarimi

Ilk test verisi:

```text
path: /system/config
type: file
content_type: config
summary: "Kernel 01 boot configuration placeholder"
tags: system, config
index_state: not_built
```

Ilk minimal Knowledge FS cevabi:

```text
hn fs meta /system/config
path=/system/config
type=file
tags=system,config
summary=Kernel 01 boot configuration placeholder
index_state=not_built
```

## Tamamlanma Kriterleri

- `kernel01/fs/metadata/`, `ai/memory_index/`, `ai/semantic_fs/`, `ai/indexer/` klasorleri hazirdir.
- Knowledge FS amaci ve HNMos FS hedefi tanimlanmistir.
- Metadata, ozet, etiket ve index plani yazilmistir.
- HNShell komutlari tanimlanmistir.
- AI Runtime Interface baglantisi Context Router ve Safety Layer uzerinden kurulmustur.
- Kernel seviyesinde guvenli metadata okuma ilkeleri belirtilmistir.

## HNMOS-CODEX-08 Durumu

CODEX-08, Knowledge FS icin ilk minimal metadata stub hedefini tasarim seviyesinde tanimlamistir:

- `/system/config` icin sabit metadata snapshot'i hedeflenir.
- `hn fs list` ve `hn fs meta /system/config` komutlari stub cevapla baslayabilir.
- `ai/indexer` icin index state enum'u hedeflenir.
- `Context Router` izinli metadata snapshot cevabi hazirlar.
- Ozet ve etiket yazma islemleri permission/audit olmadan kapali kalir.

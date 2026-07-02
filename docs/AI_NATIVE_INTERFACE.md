# AI-Native System Interface

## Hedef

Bu belge, HNMos'ta AI'nin kernel icine gomulmeden nasil OS tarafindan denetlenen bir sistem arayuzu uzerinden calisacagini tanimlar.

AI-Native System Interface Layer, HNMos'un AI destekli olmasini saglar; fakat kernel'in deterministik, kucuk ve guvenilir kalmasini engellemez.

## Dosya Yapisi

```text
docs/
|-- AI_NATIVE_INTERFACE.md
|-- ARCHITECTURE.md
|-- SYSTEM_LAYERS.md
`-- KERNEL_01_OVERVIEW.md

future/
|-- ai/
|   |-- event_stream.h
|   |-- task_graph.h
|   |-- permission_gate.h
|   |-- context_broker.h
|   `-- audit_log.h
`-- user/
    |-- hnshell/
    `-- hnlang/
```

`future/` dizini bu kademede olusturulmaz; burada yalnizca uzun vadeli dosya yonu gosterilir.

## Temel Karar

AI dogrudan kernel icine gomulmez.

Kernel su isleri yapmalidir:

- Bellek ve CPU guvenlik sinirlarini korumak.
- Syscall ve interrupt akislarini denetlemek.
- Donanim olaylarini standart cekirdek olaylarina cevirmek.
- Izin kararlarini uygulanabilir sistem sinirlarina baglamak.

AI su isleri kernel disinda yapmalidir:

- Kullanici niyetini yorumlamak.
- Gorev planlari onermek.
- Sistem baglamini ozetlemek.
- HNShell veya HNLang uzerinden calisabilecek eylem onerileri uretmek.
- Izin gerektiren islemler icin OS arayuzunden onay istemek.

## Neden AI Kernel Icine Gomulmemeli

AI'nin kernel icine gomulmemesi HNMos'un ana guvenlik ve sadelik kararidir.

Gerekceler:

- Kernel deterministik kalmalidir.
- AI ciktisi her zaman kesin ve tekrar edilebilir degildir.
- Model degisimi kernel ABI degisimi haline gelmemelidir.
- Prompt, baglam ve politika verisi kernel belleginde kontrolsuz buyumemelidir.
- Kernel hatalari tum sistemi dusurur; AI hatalari izole edilebilir olmalidir.
- Izin ve audit mekanizmasi kernel icine gomulu davranistan daha acik denetlenebilir olmalidir.

## Sistem Arayuzu Bilesenleri

AI-Native System Interface bes ana bilesenden olusur.

### Event Stream

Event Stream, sistem olaylarini AI tarafindan okunabilir ama sinirlandirilmis hale getirir.

Ornek olay turleri:

- `task.created`
- `task.completed`
- `shell.command.requested`
- `file.opened`
- `driver.keyboard.input`
- `permission.requested`
- `permission.denied`
- `kernel.panic`

Event Stream ham kernel bellegi veya rastgele pointer sunmaz. Olaylar kucuk, tipli ve loglanabilir kayitlar olarak tasarlanir.

### Task Graph

Task Graph, kullanici niyetini izlenebilir gorev adimlarina boler.

Ornek:

```text
User Intent
  -> inspect file
  -> summarize context
  -> propose command
  -> request permission
  -> execute through shell or syscall-backed service
  -> log result
```

Task Graph, AI'nin tek seferlik serbest komutlar uretmesi yerine OS tarafindan izlenen adimlar uretmesini saglar.

### Permission Gate

Permission Gate, AI'nin sistem eylemlerini hangi yetkiyle onerebilecegini veya calistirabilecegini belirler.

Izin seviyeleri:

- Observe: sadece olay ve baglam okuma.
- Suggest: kullaniciya eylem onerisinde bulunma.
- Draft: komut veya HNLang planini hazirlama.
- Request: sistem eylemi icin onay isteme.
- Execute: yalnizca acik izin verilmis dar kapsamli eylemi calistirma.

Varsayilan politika `Suggest` veya daha dusuk olmalidir. Yikici veya kalici islemler acik izin gerektirir.

### Context Broker

Context Broker, AI'ye verilecek sistem baglamini sinirlar.

Baglam kaynaklari:

- Aktif HNShell oturumu.
- Secili Knowledge FS nesneleri.
- Son sistem olaylari.
- Gorev grafigi durumu.
- Kullanici tarafindan paylasilan dosya veya komut ciktisi.

Context Broker su bilgileri otomatik olarak acmamalidir:

- Tum fiziksel bellek.
- Kernel pointer'lari.
- Gizli izin token'lari.
- Rastgele user-space process bellegi.
- Surucu tamponlari.

### Audit Log

Audit Log, AI kaynakli oneri, izin talebi ve eylemleri kaydeder.

Kayit alanlari:

- Zaman.
- Oturum.
- Istek kaynagi.
- Onerilen eylem.
- Istenen izin.
- Kullanici veya sistem karari.
- Calistirilan eylem.
- Sonuc.

Audit Log, AI-native sistemin guvenilir ve geriye donuk incelenebilir olmasi icin zorunludur.

## HNShell ile Iliski

HNShell, AI-native arayuzun insan tarafindaki ana girislerinden biridir.

AI, HNShell uzerinden:

- Komut onerisi hazirlayabilir.
- HNLang betigi taslagi uretebilir.
- Sistem durumunu ozetleyebilir.
- Kullanici onayi isteyen eylemleri gosterebilir.

HNShell, AI'nin otomatik ve kontrolsuz komut calistirma kanali olmamalidir. HNShell, izin kapisini ve audit kaydini gorunur kilan sistem yuzu olmalidir.

## HNLang ile Iliski

HNLang, AI tarafindan uretilen gorev planlarini denetlenebilir sistem betiklerine cevirmek icin kullanilabilir.

AI'nin dogrudan kernel fonksiyonu cagirmasi yerine:

```text
AI proposal
  -> HNLang task script
  -> permission check
  -> syscall-backed OS service
  -> audit log
```

Bu model, AI davranisini metin, plan ve izin uzerinden denetlenebilir hale getirir.

## Kernel ile Iliski

Kernel, AI'ye model calistirma ortami saglamaz.

Kernel'in AI-native katmana sundugu dar kapilar:

- Olay kaydi uretme.
- Syscall uzerinden izinli eylemleri uygulama.
- Process ve bellek izolasyonu saglama.
- Driver event verisini tipli olaylara cevirme.
- Panic veya guvenlik ihlallerini audit sistemine bildirme.

AI katmani kernel'in icinde degil, kernel tarafindan sinirlanan OS servisleri uzerinde calisir.

## Uzun Vadeli Hedef

Uzun vadede HNMos AI-native ama kernel-sade bir OS olmalidir.

Hedef davranis:

- Kullanici HNShell ile niyetini yazar.
- HNLang plan veya betik uretir.
- AI bu planin anlamini ve riskini aciklar.
- Permission Gate gerekli onayi ister.
- Core OS syscall ve driver event sinirlari icinde eylemi uygular.
- Audit Log tum sureci kaydeder.

## Tamamlanma Kriteri

Bu belge tamamlandiginda:

- AI'nin kernel icine gomulmeyecegi acikca belirtilmistir.
- AI-native arayuzun temel bilesenleri tanimlanmistir.
- Event, task, permission, context ve audit kavramlari ayrilmistir.
- HNShell ve HNLang'in AI ile iliskisi yazilmistir.
- Kernel'in AI katmanina yalnizca denetimli sistem kapilari sunacagi belirtilmistir.

## HNMOS-CODEX-07 HNLang Baglantisi

CODEX-07'de HNLang, AI-native arayuze denetlenebilir task script dili olarak baglanir:

- `ai.ask(...)` ifadeleri AI Runtime Interface'e dogrudan execution degil, draft/request olarak gider.
- Context Router yalnizca izinli ve sinirli baglam sunar.
- Safety Layer varsayilan `Suggest` veya `Draft` politikalarini uygular.
- AI execute yetkisi kapali kalir.

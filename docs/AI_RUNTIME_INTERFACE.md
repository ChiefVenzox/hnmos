# AI Runtime Interface

## Hedef

HNMOS-CODEX-06'nin hedefi, HNMos icinde ileride user-space seviyesinde calisabilecek AI Runtime Interface'i sifirdan tanimlamaktir.

Bu kademede gercek LLM calistirmak zorunlu degildir. Oncelik kernel, syscall/event interface, HNShell ve Task Engine arasinda AI'ye uygun olay, gorev, izin ve baglam arayuzlerini tasarlamaktir.

## Ana Karar

AI modeli kernel icine gomulmez.

AI sistemi Windows/Linux/macOS uzerinde calisan ayri bir prototip servis olarak da tasarlanmaz.

Hedef mimari, HNMos'un kendi Core OS katmanlari olgunlastikca user-space seviyesinde calisabilecek bir AI runtime arayuzudur. Kernel 01'deki COM2 OpenAI bridge yalnizca QEMU gelistirme tasimasidir: task, policy, workspace ve UI HNMos icinde kalir; host script kernelin veya runtime'in yerine gecmez. Native network/TLS hazir oldugunda bu tasima adapter'i kaldirilir.

## Dosya Yapisi

```text
ai/
|-- runtime_interface/
|-- task_engine/
|-- context_router/
|-- safety_layer/
`-- model_runtime_spec/

docs/
|-- AI_RUNTIME_INTERFACE.md
|-- TASK_ENGINE.md
`-- MODEL_RUNTIME_SPEC.md
```

## Mimari Akis

```text
Kernel 01
  -> Syscall / Event Interface
  -> HNShell
  -> Task Engine
  -> AI Runtime Interface
  -> Future Local Model Runtime
```

Kernel sadece guvenli sistem sinirlarini ve olay kaynaklarini saglar. HNShell kullanici niyetini alir. Task Engine niyeti izlenebilir gorevlere cevirir. AI Runtime Interface, izinli baglam ve guvenlik katmani uzerinden model runtime'a istek hazirlar.

## AI Runtime Interface Gorevi

AI Runtime Interface su isleri yapar:

- HNShell veya Task Engine tarafindan gelen istekleri kabul eder.
- Istege uygun sistem baglamini Context Router'dan ister.
- Safety Layer'a izin ve risk kontrolu yaptirir.
- Gercek model yoksa stub/model adapter ile kontrollu cevap uretir.
- Gelecekte local model runtime'a standart istek formatinda baglanir.
- Sonucu Task Engine'e, HNShell'e veya event/audit katmanina geri verir.

AI Runtime Interface su isleri yapmaz:

- Kernel fonksiyonlarini dogrudan cagirmak.
- Syscall, permission veya audit sinirlarini atlamak.
- Sistem komutlarini otomatik calistirmak.
- Kernel memory, raw driver buffer veya kernel pointer okumak.
- Host OS servislerine HNMos'un asil runtime'i gibi baglanmak.

## Model Neden Kernel Icine Gomulmeyecek

Gercek model kernel icine gomulmez cunku:

- Kernel deterministik ve kucuk kalmalidir.
- Model bellek kullanimi buyuk ve degiskendir.
- Prompt, baglam ve cevap uretimi kernel guvenlik sinirini bulaniklastirir.
- Model hatasi tum sistemi dusurmemelidir.
- Model degisimi kernel ABI degisimi olmamalidir.
- AI kararlarinin permission ve audit katmanlariyla denetlenmesi gerekir.

HNMos'ta AI, kernel'in parcasi degil, OS tarafindan sinirlandirilmis user-space servisidir.

## User-Space AI Runtime Hedefi

Uzun vadede AI Runtime su sekilde calismalidir:

```text
user-space ai runtime process
  -> syscall ile event/context okuma
  -> safety layer policy check
  -> local model runtime adapter
  -> task response
  -> audit log
```

Bu hedefe ulasmak icin once syscall, event interface, task engine ve permission layer olgunlasmalidir.

## Context Router Mantigi

Context Router, AI'ye verilecek baglami sinirlar.

Ilk kaynaklar:

- Aktif HNShell komutu.
- Task Engine gorev kaydi.
- Guvenli event ozeti.
- Build/kernel durum ozeti.
- Izin verilmis Knowledge FS nesneleri.

Asla dogrudan acilmamasi gerekenler:

- Kernel pointer'lari.
- Fiziksel bellek.
- Page table icerigi.
- Rastgele process bellegi.
- Gizli izin token'lari.
- Ham driver buffer'lari.

Context Router, "AI neyi gorebilir?" sorusunun tek kapisi olmalidir.

## Safety Layer Mantigi

Safety Layer, AI kaynakli her oneriyi veya eylem istegini siniflandirir.

Izin seviyeleri:

- Observe: sadece durum ve olay ozeti okuma.
- Suggest: kullaniciya metinsel oneride bulunma.
- Draft: HNShell/HNLang komut taslagi hazirlama.
- Request: sistem eylemi icin izin isteme.
- Execute: yalnizca acikca izin verilmis, dar kapsamli eylemi calistirma.

Varsayilan seviye `Suggest` olmalidir. Kalici, yikici veya guvenlik etkisi olan eylemler acik izin ve audit kaydi olmadan calismaz.

## AI'nin Sistem Komutlarini Dogrudan Calistirmamasi

AI sistem komutlarini dogrudan calistirmamalidir.

Dogru akış:

```text
AI suggestion
  -> Task Engine task proposal
  -> Safety Layer permission check
  -> HNShell/HNLang visible draft
  -> user or policy approval
  -> syscall-backed OS service
  -> audit/event log
```

Bu ilke, AI'nin shell veya syscall kapisini gizlice atlamasini engeller.

## Ilk OS Ici Stub / Model Adapter Yapisi

Ilk asamada gercek LLM yerine stub adapter kullanilir.

Stub adapter gorevleri:

- `hn ai status` icin `not loaded` cevabi vermek.
- Basit task istegine sabit, denetlenebilir cevap dondurmek.
- Model runtime henuz yoksa bunu acikca bildirmek.
- Safety Layer ve Context Router akisini test etmek.

Ilk adapter cevap ornekleri:

```text
AI Runtime Interface: not loaded
model adapter: stub
execution: disabled
```

## Uzun Vadeli Local Model Runtime Plani

Uzun vadede local model runtime:

- User-space process olarak calisir.
- HNMos permission layer tarafindan sinirlanir.
- Context Router'dan sadece izinli baglam alir.
- Task Engine'e sadece oneriler veya izinli task cevaplari dondurur.
- Model dosyalarini Knowledge FS veya ayri model store uzerinden yukler.
- Audit log olmadan sistem eylemi baslatamaz.

## Tamamlanma Kriterleri

- `ai/runtime_interface/`, `ai/task_engine/`, `ai/context_router/`, `ai/safety_layer/`, `ai/model_runtime_spec/` klasorleri hazirdir.
- AI modelinin kernel icine gomulmeyecegi belirtilmistir.
- AI'nin host prototip servis olarak tasarlanmayacagi belirtilmistir.
- User-space AI runtime hedefi yazilmistir.
- Context Router ve Safety Layer sorumluluklari tanimlanmistir.
- AI'nin sistem komutlarini dogrudan calistirmamasi ilkesi kayda gecmistir.
- Stub/model adapter ilk asama olarak tanimlanmistir.

## HNMOS-CODEX-06 Durumu

CODEX-06, AI Runtime Interface'in ilk minimal iskeletini tasarim seviyesinde tanimlamistir:

- `hn ai status` komutu icin stub cevabi planlanmistir.
- Task Engine icin `task_request` ve `task_result` hedefleri belirlenmistir.
- Context Router icin sabit kernel/shell durum ozeti hedeflenmistir.
- Safety Layer icin varsayilan `Suggest` politikasi tanimlanmistir.
- AI'nin execute yetkisi kapali tutulur.

HNLang kademesinde AI cagrilari `ai.ask(...)` gibi ifadelerle yalnizca draft/request akisi uzerinden AI Runtime Interface'e ulasir.

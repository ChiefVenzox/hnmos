# Model Runtime Spec

## Hedef

Bu belge, HNMos'un uzun vadeli Future Local Model Runtime hedefini tanimlar.

CODEX-06'da gercek LLM calistirilmaz. Bu belge, ileride HNMos icinde user-space seviyesinde calisabilecek local model runtime icin sinirlari ve adapter sozlesmesini belirler.

## Dosya Yapisi

```text
ai/
`-- model_runtime_spec/
    |-- model_adapter.h
    |-- model_request.h
    |-- model_response.h
    |-- model_capabilities.h
    `-- model_manifest.h
```

Bu dosyalar gelecekteki uygulama hedefidir.

## Model Runtime Nerede Calisir

Model runtime kernel icinde calismaz.

Model runtime host OS uzerinde ayri HNMos prototip servisi olarak da tasarlanmaz.

Uzun vadeli hedef:

```text
HNMos user-space model runtime process
  -> model adapter ABI
  -> local model weights
  -> bounded context input
  -> structured response
```

## Model Adapter Sorumlulugu

Model adapter, AI Runtime Interface ile gercek model runtime arasindaki dar sozlesmedir.

Adapter su isleri yapar:

- Model durumunu bildirir.
- Model kapasitesini bildirir.
- Izinli context paketini modele gonderir.
- Model cevabini yapisal response formatina cevirir.
- Timeout, memory pressure ve hata durumlarini raporlar.

Adapter su isleri yapmaz:

- Kernel syscall'larini dogrudan calistirmak.
- HNShell komutlarini otomatik execute etmek.
- Context Router disindan baglam okumak.
- Safety Layer'i atlamak.

## Ilk Stub Adapter

CODEX-06 ve CODEX-07 icin ilk adapter gercek model calistirmadan sabit cevap verir.

Ilk status cevabi:

```text
model.runtime = stub
model.loaded = false
execution.enabled = false
```

Ilk request cevabi:

```text
response.kind = status
response.text = "AI Runtime Interface: not loaded"
confidence = 0
requires_permission = false
```

Bu stub, HNShell ve Task Engine entegrasyonunu test etmek icindir.

## Model Request Formati

Ilk request alanlari:

```text
request_id
task_id
request_kind
allowed_context
safety_level
max_tokens
timeout_ms
```

`allowed_context`, Context Router tarafindan hazirlanir. Model runtime kendisi sistemden ek baglam cekmez.

## Model Response Formati

Ilk response alanlari:

```text
request_id
task_id
response_kind
text
proposed_action
requires_permission
risk_level
error_code
```

`proposed_action` varsa bile otomatik calismaz. Task Engine ve Safety Layer bunu ayri bir izin akisina cevirir.

## Local Model Runtime Uzun Vadeli Plani

Uzun vadeli asamalar:

1. Stub adapter.
2. Basit rule-based local responder.
3. Kucuk yerel model icin user-space runtime.
4. Model manifest ve model store.
5. Context budget ve memory pressure yonetimi.
6. Permission ve audit zorunlu execution gate.
7. HNLang task draft uretimi.

## Kaynak ve Bellek Sinirlari

Model runtime:

- Kendi user-space adres alaninda calismalidir.
- Kernel memory'ye dogrudan erisememelidir.
- Memory pressure durumunda task'i iptal edebilmelidir.
- Scheduler tarafindan normal process gibi zamanlanmalidir.
- Dosya/model yukleme islemlerini permission layer ile yapmalidir.

## Guvenlik Ilkesi

Model response bir sistem eylemi degildir.

```text
model response
  -> AI Runtime Interface
  -> Safety Layer
  -> Task Engine
  -> HNShell/HNLang visible draft
  -> permission
  -> syscall-backed execution
```

Bu zincir kirilirsa model runtime guvenli kabul edilmez.

## Tamamlanma Kriterleri

- Model runtime'in kernel icinde calismayacagi belirtilmistir.
- Host prototip servis hedefi reddedilmistir.
- User-space local model runtime hedefi tanimlanmistir.
- Stub adapter ilk asama olarak yazilmistir.
- Request/response alanlari ve safety sinirlari belirlenmistir.

## HNMOS-CODEX-07 HNLang Baglantisi

CODEX-07'de HNLang, model runtime'a dogrudan baglanmaz. Baglanti AI Runtime Interface uzerinden planlanir:

1. `ai.ask(...)` ifadesi model request degil, AI Runtime request taslagidir.
2. Stub adapter `AI Runtime Interface: not loaded` cevabini dondurebilir.
3. `execution.enabled = false` politikasi korunur.
4. Gercek model yukleme sonraki kademelere birakilir.

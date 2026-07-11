# HNLang AI Studio Integration

HNMos AI Studio, gercek provider baglantisini HNLang kod uretiminden ve OS yetkisinden ayirir. OpenAI API anahtari modele erisim saglar; bu kimlik bilgisi HNMos sistem yetkisi vermez. ChatGPT aboneligi OpenAI API kullanimindan ayri oldugu icin `login with subscription` API kimlik dogrulamasi olarak kullanilmaz.

## Yuzey

Launcher'daki `5 AI Studio` ikonu su alanlari acar:

- ChatGPT subscription/API ayrimi hakkinda acik durum bilgisi.
- Maskeli ve gecici API key girisi.
- `/workspace/main.hn` RAM dosyasini gosteren IDE yuzeyi.
- Gercek provider promptu, review ve build-request akisi.
- Aktif workspace policy ozeti.

## Calisan Provider Akisi

Kernel 01 dogrudan ag/TLS yigini icermedigi icin QEMU gelistirme tasimasi kullanilir:

```text
AI Studio
  -> HNAI1 framed request
  -> COM2 (IRQ3)
  -> private Unix socket
  -> tools/ai/openai_bridge.py
  -> HTTPS POST /v1/responses
  -> framed response over COM2
  -> policy check
  -> /workspace/main.hn
```

API key iki sekilde saglanabilir:

```text
1. AI Studio -> K -> key yaz -> ENTER
2. Host ortaminda OPENAI_API_KEY ayarla
```

Studio girdisi maskelenir. Input debug key girisi boyunca kapatilir. Anahtar COM2 bridge'e aktarildiktan sonra HNMos tamponu sifirlanir; dosyaya, manifest'e veya log'a yazilmaz. Unix socket yalnizca sahibi tarafindan okunabilecek izinlerle acilir.

Cross compiler ve QEMU kuruluysa:

```sh
export OPENAI_API_KEY="..."       # opsiyonel; key Studio icinden de girilebilir
export OPENAI_MODEL="gpt-5.4-mini"
./run-qemu-ai.sh
```

QEMU acildiktan sonra:

```text
5 -> AI Studio
K -> API key girisi -> ENTER
G -> HNLang uygulama istegini yaz -> ENTER
```

Provider cevabi geldiginde IRQ3 kernel'i uyandirir, task tamamlanir ve cevap `/workspace/main.hn` icine yazilir. IDE ayni RAM dosyasini yeniden cizer. Dosya reboot sonrasinda kalici degildir.

## Degistirilemez HNMos Siniri

AI Studio deny-by-default calisir. `kernel01/`, boot, policy, driver, audit ve sistem ayarlari icin AI yetkisi yoktur. Credential eklemek bu karari degistiremez.

| Capability | Karar | Sinir |
|---|---|---|
| `read.workspace` | allow | Yalnizca `/workspace/` |
| `write.workspace.hnlang` | allow | Yalnizca `/workspace/*.hn` draft |
| `read.build.log` | allow | Yalnizca workspace loglari |
| `run.workspace.build` | approval | Kullanici onayi olmadan calismaz |
| `network.provider` | approval | Yapilandirilmis HTTPS provider endpoint |
| system/kernel/boot/policy/driver/audit | deny | Istisna yok |

Path policy `..` traversal girisimlerini ve `/workspace/` disindaki yazma hedeflerini reddeder. Build dugmesi komut calistirmaz; yalnizca approval request uretir. Provider cevabi hicbir zaman shell komutu, syscall veya driver islemi olarak dogrudan uygulanmaz.

## HNLang AI Profili

Her `HNM_AI_TASK_HNLANG` kaydi provider'a su dil sozlesmesini tasir:

```text
Generate HNLang workspace drafts only; never modify kernel, boot,
policy, drivers, or audit state.
```

Profil yedi typed primitive sunar:

```text
intent.plan      UserIntent  -> TaskPlan
code.generate    TaskPlan    -> HNLangDraft
code.refactor    HNLangDraft -> HNLangDraft
policy.validate  HNLangDraft -> PolicyReport
build.request    HNLangDraft -> ApprovalRequest
test.explain     BuildReport -> Explanation
preview.render   HNLangDraft -> StudioPreview
```

Verimli kombinasyonlar sinirli recipe olarak birlestirilir:

```text
app.create   = intent.plan > code.generate > policy.validate > preview.render
app.optimize = code.refactor > policy.validate > build.request > test.explain
error.repair = intent.plan > code.refactor > policy.validate > build.request > test.explain
ui.compose   = intent.plan > code.generate > policy.validate > preview.render
```

Her recipe sabit maksimum adim sayisina sahiptir. Dongusuz, typed ve review edilebilir kombinasyonlar context boyutunu sinirlar ve primitive'lerin yeniden kullanilmasini saglar.

## Kalan Native Yol

QEMU COM2 bridge calisan ilk provider entegrasyonudur. Host bridge'i kaldirip HTTPS'i tamamen HNMos icinde calistirmak icin gereken sonraki katmanlar:

1. User-space network, DNS, TCP ve TLS runtime.
2. Native provider adapter ve endpoint allowlist.
3. Sifreli user secret store.
4. HNLang parser ve type checker.
5. Kalici workspace VFS ile approval/audit baglantisi.

Mevcut bridge gercek OpenAI API istegi yapar, ancak QEMU gelistirme tasimasidir; son urun native network/TLS katmanina gecmelidir.

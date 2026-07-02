# AI Permissions

## Hedef

HNMOS-CODEX-11'in hedefi, HNMos icinde AI'nin hangi sistem bilgilerini okuyabilecegini, hangi eylemleri onerebilecegini ve hangi eylemler icin kullanici onayi gerekecegini tanimlayan izin modelini tasarlamaktir.

Ana ilke:

```text
AI onerir.
Task Engine degerlendirir.
Permission System kontrol eder.
Kullanici kritik islemleri onaylar.
OS uygular ve loglar.
```

AI, kernel icinde serbest calisan bir yetkili bilesen degildir. AI Runtime user-space hedefidir ve sistemle yalnizca izinli OS arayuzleri uzerinden etkilesir.

## Dosya Yapisi

```text
ai/
|-- safety_layer/
|-- permission_manifest/
|-- action_audit/
`-- policy_engine/

docs/
|-- AI_PERMISSIONS.md
|-- SECURITY_MODEL.md
`-- AUDIT_LOGS.md
```

- `ai/safety_layer/`: izin, risk ve kullanici onayi kararlarinin AI tarafindaki guvenlik katmani.
- `ai/permission_manifest/`: AI runtime, HNShell task ve HNLang script izin tanimlari.
- `ai/action_audit/`: kritik eylem ve karar kayitlari icin audit hedefi.
- `ai/policy_engine/`: policy kurallarini degerlendiren karar katmani.

## AI Izin Modeli

AI izin modeli capability tabanli olmalidir. AI'ye genel "admin" yetkisi verilmez. Her task, yapmak istedigi is icin belirli izinler ister.

Izin siniflari:

```text
read.system_status
read.events.public
read.fs.metadata
read.fs.content
write.fs.file
delete.fs.file
run.shell.command
change.system.setting
inspect.kernel.state
control.driver
network.send
```

Varsayilan karar:

```text
deny by default
```

Yani bir task veya AI eylemi acikca izin istemiyorsa ve policy tarafindan izinli degilse calismaz.

## Permission Manifest Yapisi

Her AI task, HNShell komutu veya HNLang scripti bir permission manifest ile birlikte degerlendirilmelidir.

Ornek manifest:

```json
{
  "manifest_version": 1,
  "actor": "ai.runtime.local",
  "task_id": "task-42",
  "intent": "fix_kernel_build_errors",
  "requested_permissions": [
    {
      "capability": "read.fs.content",
      "scope": "/kernel01",
      "reason": "inspect source files"
    },
    {
      "capability": "run.shell.command",
      "scope": "make verify",
      "reason": "verify build after proposed changes"
    }
  ],
  "risk_level": "medium",
  "requires_user_approval": true,
  "expires_after_ticks": 5000
}
```

Manifest kurallari:

- Scope dar tutulur.
- Reason bos birakilmaz.
- Kritik izinler sureli olur.
- Manifest task id ile audit log'a baglanir.
- AI kendi manifestini tek basina onaylayamaz.

## Risk Seviyeleri

Risk seviyeleri policy engine tarafindan kullanilir:

```text
none
low
medium
high
critical
forbidden
```

- `none`: sadece aciklama veya UI metni.
- `low`: public event, sistem status veya read-only metadata.
- `medium`: dosya icerigi okuma, build log inceleme, shell komutu onerme.
- `high`: dosya yazma, build komutu calistirma, sistem ayari onerme.
- `critical`: dosya silme, disk yazma, kernel/boot alani degistirme, network gonderimi.
- `forbidden`: kernel memory write, driver control, kullanici onayi olmadan destructive islem.

## Kullanici Onay Mekanizmasi

Kritik islem akisi:

```text
AI proposal
  -> Task Engine
  -> Permission Manifest
  -> Policy Engine
  -> Approval Prompt
  -> User Decision
  -> Action Audit
  -> Execution
```

Onay prompt'u acik olmalidir:

```text
permission required:
- actor: ai.runtime.local
- task: fix_kernel_build_errors
- action: write.fs.file
- scope: /kernel01/kernel/main.c
- risk: high
- reason: apply build fix

approve? [y/N]
```

Kullanici cevap vermeden high veya critical islem uygulanmaz.

## Yasakli Islemler

AI icin dogrudan yasakli islemler:

- Kernel memory alanina yazmak.
- Interrupt, paging veya scheduler durumunu dogrudan degistirmek.
- Driver register veya port I/O uzerinden donanim kontrol etmek.
- Kullanici onayi olmadan dosya silmek.
- Kullanici onayi olmadan sistem ayari degistirmek.
- Kullanici onayi olmadan disk yazma islemi yapmak.
- Gizli/sensitive event payload okumak.
- Audit log'u silmek veya degistirmek.
- Kendi izin manifestini onaylamak.

Bu islemler AI tarafindan sadece onerilebilir veya aciklanabilir. Uygulama, OS policy ve kullanici onayi tarafindan kontrol edilir.

## Izinli Islemler

AI'nin izne bagli veya varsayilan guvenli yapabilecegi islemler:

- Public sistem durumunu ozetlemek.
- Public event snapshot okumak.
- Dosya metadata'si okumak.
- Kullanici tarafindan saglanan metni ozetlemek.
- Hata icin duzeltme plani onermek.
- Komut onermek.
- Task planini Task Engine'e kaydetmek.
- Risk ve izin gereksinimlerini aciklamak.

Dosya icerigi okuma, komut calistirma ve dosya yazma gibi islemler manifest ve policy kontrolu gerektirir.

## Kernel/User-Space Sinirlari

Kernel tarafinda kalacak guvenlik sinirlari:

- Syscall kapilari.
- Event snapshot izinleri.
- Kernel memory izolasyonu.
- Driver control engelleri.
- Panic ve fault durumlari.

User-space hedefleri:

- AI Runtime.
- Task Engine.
- Permission Manifest loader.
- Policy Engine.
- Action Audit writer.
- HNShell approval UI.

AI Runtime kernel icine gomulmez. Kernel sadece sinirli ve denetlenebilir sistem arayuzleri sunar.

## Ornek Senaryolar

### Senaryo 1: Dosya Silme

Kullanici:

```text
Gereksiz log dosyalarini temizle.
```

AI:

```text
delete.fs.file /logs/*
```

Policy sonucu:

```text
risk: critical
requires_user_approval: true
decision: pending_user
```

Kullanici onay vermezse islem yapilmaz.

### Senaryo 2: Sistem Ayari Degistirme

AI, timer frekansini degistirmeyi onerir.

Policy sonucu:

```text
capability: change.system.setting
risk: high
approval: required
```

Task Engine sadece onayli ayar degisikligini uygular.

### Senaryo 3: Kernel Inspect

AI bellek durumunu yorumlamak ister.

Izinli yol:

```text
inspect.kernel.state scope=memory.summary
```

Yasak yol:

```text
kernel.memory.write
```

Kernel inspect snapshot read-only olmalidir.

## Tamamlanma Kriterleri

- AI izin modeli tanimlanmistir.
- Permission manifest yapisi yazilmistir.
- Risk seviyeleri belirlenmistir.
- Kullanici onay mekanizmasi aciklanmistir.
- Yasakli ve izinli islemler ayrilmistir.
- Kernel/user-space guvenlik sinirlari tanimlanmistir.
- Ornek guvenlik senaryolari yazilmistir.

## Permission Uygulama Notu

Bu izin modeli ilk minimal uygulama taslagina indirilirken hedef:

1. Permission capability enum'u.
2. Risk level enum'u.
3. Manifest struct taslagi.
4. Policy decision struct taslagi.
5. HNShell tarafinda approval prompt stub'u.
6. Action audit icin append-only log taslagi.

# Audit Logs

## Hedef

Bu belge, HNMos icinde kritik AI destekli eylemlerin, kullanici onaylarinin ve policy kararlarinin nasil loglanacagini tanimlar.

Audit log, AI Permission Layer'in hafizasidir. Her kritik islem izlenebilir olmalidir.

## Audit Ilkeleri

- Append-only olmalidir.
- Kritik eylemler zorunlu loglanmalidir.
- AI audit log'u silemez veya degistiremez.
- Log kaydi task id, actor, capability, karar ve sonuc tasimalidir.
- Kullanici onayi veya reddi acikca kaydedilmelidir.
- Kernel seviyesi kritik olaylar icin minimum log alani ayrilmalidir.

## Log Kaydi Formati

Ornek JSON format:

```json
{
  "audit_version": 1,
  "event_id": 128,
  "tick": 9042,
  "actor": "ai.runtime.local",
  "user": "root",
  "task_id": "task-42",
  "action": "write.fs.file",
  "scope": "/kernel01/kernel/main.c",
  "risk_level": "high",
  "policy_decision": "require_user_approval",
  "user_decision": "approved",
  "result": "executed",
  "message": "AI proposed kernel source edit; user approved; task engine executed."
}
```

Kernel veya erken boot icin daha kucuk struct formati:

```c
struct hnm_audit_event {
    unsigned int event_id;
    unsigned int tick;
    unsigned int task_id;
    unsigned short action;
    unsigned short risk_level;
    unsigned short policy_decision;
    unsigned short user_decision;
    unsigned int scope_hash;
    unsigned int result;
};
```

## Audit Event Tipleri

```text
policy.evaluate
permission.request
permission.approve
permission.deny
action.propose
action.execute
action.reject
action.fail
context.read
audit.rotate
security.violation
```

`audit.rotate` ileride log saklama sistemi icin kullanilir; AI tarafindan baslatilamaz.

## Zorunlu Loglanacak Islemler

- Dosya yazma.
- Dosya silme.
- Sistem ayari degistirme.
- Shell komutu calistirma.
- Disk yazma.
- Network gonderimi.
- Kernel/boot dosyasi degistirme.
- Permission manifest onayi.
- Policy deny karari.
- Yasakli action denemesi.

## Opsiyonel veya Ozet Log

- Public event okuma.
- Metadata listeleme.
- UI panel acma.
- Yardim metni uretme.
- Task plan taslagi.

Bu kayitlar debug veya session log seviyesinde tutulabilir.

## Kullanici Onay Kaydi

Onay kaydi sadece "evet" veya "hayir" yazmamali; neyin onaylandigini da tasimalidir.

Ornek:

```text
task=task-42
actor=ai.runtime.local
action=delete.fs.file
scope=/logs/old/*
risk=critical
decision=denied
user=root
```

Kullanici reddederse eylem uygulanmaz ve result `rejected_by_user` olur.

## Audit Log Guvenligi

Audit log icin kurallar:

- AI audit log yazabilir gibi gorunmez; Task Engine veya OS audit servisi yazar.
- AI audit log'u silemez.
- Log kaydi sonradan degistirilemez.
- Disk FS hazir degilse ring buffer veya serial audit ciktisi kullanilir.
- Panic durumunda son kritik audit kayitlari ekrana veya serial'a basilebilir.

## Kernel/User-Space Audit Ayrimi

Kernel tarafinda:

- Fault.
- Panic.
- Critical syscall deny.
- Driver access deny.
- Kernel snapshot request.

User-space tarafinda:

- AI proposal.
- Manifest request.
- Policy decision.
- User approval.
- Task execution result.

Ilk Kernel 01 asamasinda audit log tam dosya sistemine yazilmak zorunda degildir. Ring buffer ve serial log yeterlidir.

## Ornek Senaryolar

### Dosya Silme Reddedildi

```json
{
  "event_id": 9,
  "actor": "ai.runtime.local",
  "task_id": "task-7",
  "action": "delete.fs.file",
  "scope": "/system/cache/*",
  "risk_level": "critical",
  "policy_decision": "require_user_approval",
  "user_decision": "denied",
  "result": "rejected_by_user"
}
```

### Public Event Okundu

```json
{
  "event_id": 10,
  "actor": "ai.runtime.local",
  "task_id": "task-8",
  "action": "read.events.public",
  "scope": "event_snapshot.public",
  "risk_level": "low",
  "policy_decision": "allow",
  "result": "snapshot_returned"
}
```

### Yasakli Kernel Erisimi

```json
{
  "event_id": 11,
  "actor": "ai.runtime.local",
  "task_id": "task-9",
  "action": "kernel.memory.write",
  "scope": "kernel",
  "risk_level": "forbidden",
  "policy_decision": "deny",
  "result": "security_violation"
}
```

## Tamamlanma Kriterleri

- Audit log amaci tanimlanmistir.
- JSON ve struct formatlari yazilmistir.
- Zorunlu loglanacak kritik islemler listelenmistir.
- Kullanici onay kaydi formati belirlenmistir.
- AI'nin audit log'u silemeyecegi veya degistiremeyecegi belirtilmistir.
- Kernel/user-space audit ayrimi tanimlanmistir.

## Audit Uygulama Notu

Audit icin ilk minimal append-only ring buffer ve serial audit output taslagi hedeflenmelidir. FS hazir degilse audit log bellek ici tutulur ve QEMU serial ciktisina basilabilir.

# Security Model

## Hedef

Bu belge, HNMos'un kernel, user-space servisleri, HNShell, Task Engine ve AI Runtime arasindaki guvenlik sinirlarini tanimlar.

Ana kural:

```text
Kullanici son yetkilidir.
AI dogrudan sistem yetkilisi degildir.
Kernel en dar ve zorunlu yetki yuzeyini saglar.
User-space servisler policy ile sinirlanir.
```

## Sistem Katmanlari

```text
Kernel 01
  -> syscall/event snapshot
  -> HNShell
  -> Task Engine
  -> Permission Manifest
  -> Policy Engine
  -> Safety Layer
  -> AI Runtime Interface
  -> Future Local Model Runtime
```

AI, bu zinciri atlayamaz. Donanim, kernel memory, dosya sistemi yazma ve komut calistirma yollarina sadece policy kontrollu task uzerinden ulasir.

## Guvenlik Prensipleri

- Deny by default.
- Least privilege.
- Explicit scope.
- User approval for destructive or privileged actions.
- Append-only audit for critical actions.
- Kernel isolation before AI capability.
- AI proposes, OS decides, user authorizes.

## Kernel Guvenlik Siniri

Kernel 01 su alanlari korumali kabul eder:

- Kernel code ve data segmentleri.
- Interrupt descriptor table.
- Paging yapilari.
- Scheduler state.
- Syscall dispatch tablosu.
- Driver I/O portlari ve register erisimi.
- Panic ve fault handling.

AI Runtime bu alanlara dogrudan erisemez.

Izinli kernel bilgisi snapshot biciminde verilir:

```text
memory.summary
interrupt.status
timer.tick_count
event.public_snapshot
fs.metadata_summary
```

Snapshot read-only olmalidir ve sensitive payload tasimamalidir.

## User-Space Guvenlik Siniri

Uzun vadede HNShell, Task Engine, Policy Engine ve AI Runtime user-space servisler olmalidir.

Her servis:

- Kendi capability seti ile baslar.
- Kernel syscall kapisindan sinirli is yapar.
- Audit kaydi olmadan kritik eylem yapamaz.
- AI tarafindan gelen oneriyi komut olarak otomatik calistirmaz.

## Task Engine'in Rolu

Task Engine, AI ile sistem arasindaki uygulama kapisidir.

Gorevleri:

- Kullanici istegini task request'e cevirir.
- AI onerisini action proposal olarak alir.
- Permission manifest ister veya uretir.
- Policy Engine sonucunu bekler.
- Kullanici onayi gerekiyorsa HNShell/UI'ye sorar.
- Onayli eylemi OS servislerine yollar.
- Sonucu audit log ile baglar.

Task Engine olmadan AI eylemi uygulanmaz.

## Policy Engine'in Rolu

Policy Engine, izin ve risk kararini uretir.

Input:

```text
actor
task_id
requested_capability
scope
reason
current_user
system_state
risk_context
```

Output:

```text
decision: allow | deny | require_user_approval | require_elevated_approval
risk_level
audit_required
sanitized_scope
message
```

Ornek karar:

```json
{
  "decision": "require_user_approval",
  "risk_level": "high",
  "audit_required": true,
  "sanitized_scope": "/kernel01/kernel/main.c",
  "message": "AI proposed writing a kernel source file."
}
```

## Safety Layer'in Rolu

Safety Layer, AI Runtime tarafina yakin duran guvenlik filtresidir.

Gorevleri:

- AI output'unu action proposal bicimine indirger.
- Yasakli action'lari dogrudan reddeder.
- Risk seviyesini policy engine'e iletir.
- Gizli baglamin modele verilmesini engeller.
- Kullanici onayi olmadan execution'a gecis yapmaz.

Safety Layer tek basina yetkili karar mercii degildir; Policy Engine ve kullanici onayi ile birlikte calisir.

## Yasakli Yollar

Asagidaki yollar tasarim geregi yasaktir:

```text
AI -> Kernel direct call
AI -> Driver control
AI -> Raw disk write
AI -> File delete without approval
AI -> System setting write without approval
AI -> Audit log modification
AI -> Permission self-approval
```

Bu yollar docs ve kod seviyesinde guvenlik ihlali olarak kabul edilir.

## Izinli Yollar

Guvenli yol:

```text
AI proposal
  -> Safety Layer
  -> Task Engine
  -> Permission Manifest
  -> Policy Engine
  -> User Approval if needed
  -> OS service/syscall
  -> Audit Log
```

Read-only ve public bilgi icin daha kisa yol olabilir:

```text
AI context request
  -> Context Router
  -> Policy Engine
  -> sanitized snapshot
```

## Risk Matrisi

```text
Action                         Risk       User approval
read public event              low        no
read fs metadata               low        no or session policy
read file content              medium     maybe
run build verify               medium     maybe
write source file              high       yes
change system setting          high       yes
delete file                    critical   yes
write disk block               critical   yes
driver control                 forbidden  no direct AI path
kernel memory write            forbidden  no direct AI path
```

## Kullanici Son Yetkili

HNMos, AI destekli olabilir; fakat son yetki kullanicidadir.

Kullanici:

- Eylemi onaylayabilir.
- Eylemi reddedebilir.
- Izin kapsamini daraltabilir.
- Session izinlerini iptal edebilir.
- Audit log'u inceleyebilir.

AI bu kararlari gecersiz kilamaz.

## Tamamlanma Kriterleri

- Kernel/user-space guvenlik sinirlari tanimlanmistir.
- Task Engine, Policy Engine ve Safety Layer rolleri ayrilmistir.
- Yasakli ve izinli yollar yazilmistir.
- Risk matrisi hazirlanmistir.
- Kullanici son yetkili ilkesi sistem modeline yerlestirilmistir.

## Security Uygulama Notu

Bu model ilk code-level sozlesmelere cevrilirken hedef policy decision struct, permission enum ve audit event formatinin kernel/user-space sinirina uygun C header veya taslak dokuman olarak yazilmasidir.

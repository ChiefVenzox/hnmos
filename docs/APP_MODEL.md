# HNMos App Model

## Hedef

Bu belge, HNMos uzerinde calisacak uygulamalarin hangi sinirlar icinde var olacagini, nasil baslatilacagini ve OS servisleriyle nasil etkilesecegini tanimlar.

HNMos uygulama modeli host OS uygulama modeli degildir. Hedef, HNMos user-space icinde calisan sistem araclari, gorev uygulamalari ve AI destekli yardimci uygulamalardir.

## Uygulama Modeli

HNMos uygulamasi su parcalardan olusur:

```text
app package
  -> app.manifest.json
  -> entry file
  -> optional resources
  -> requested permissions
  -> shell commands
  -> UI mode
```

Uygulama baslatma HNShell, Command Palette veya ileride Window Manager uzerinden olabilir. Baslatma kaynagi farkli olsa da manifest, izin ve audit kurallari aynidir.

## Uygulama Tipleri

```text
system_tool
task_app
ai_assistant_app
driver_agent
dev_tool
ui_panel
```

### system_tool

Sistem durumunu veya public event snapshot'larini okuyan arac.

Ornek:

```text
hn-monitor
```

### task_app

Kullanici istegini gorev akisi olarak isleyen uygulama.

Ornek:

```text
hn-notes
hn-files
```

### ai_assistant_app

AI Runtime Interface kullanan, fakat eylemleri dogrudan uygulamayan yardimci uygulama.

Ornek:

```text
hn-ai-lab
```

### driver_agent

Driver Event Layer uzerinden izinli snapshot okuyan user-space agent.

Ornek:

```text
hn-camera-agent
```

### dev_tool

Build, verify, debug ve kod inceleme gorevleri icin arac.

Ornek:

```text
hn-builder
hn-dev-studio
```

## Manifest Zorunlulugu

HNMos'ta uygulama manifest olmadan baslatilmaz.

Manifest su guvenlik sorularini cevaplar:

- Bu app kim?
- Hangi runtime ile calisir?
- Hangi entry dosyasindan baslar?
- Hangi izinleri ister?
- Hangi shell komutlariyla calisir?
- AI kullanir mi?
- UI modu nedir?

Eksik veya gecersiz manifest varsayilan olarak reddedilir.

## Permission Baglantisi

App permission listesi, AI Permission Layer ile ayni capability dilini kullanir.

Ornek:

```text
read.system_status
read.events.public
read.fs.metadata
read.fs.content
write.fs.file
run.shell.command
```

Baslatma sirasinda policy sonucu:

```text
allow
deny
require_user_approval
require_elevated_approval
```

High veya critical riskli izinler icin kullanici son yetkilidir.

## HNShell App Akisi

```text
hn app run hn-ai-lab
  -> locate app package
  -> parse manifest
  -> validate runtime
  -> evaluate permissions
  -> ask user if needed
  -> create task record
  -> launch runtime
  -> write audit log
```

HNShell uygulamayi kernel yetkisiyle calistirmaz. Uygulama user-space task olarak calisir.

## Command Palette App Akisi

Kullanici:

```text
Sistemdeki yavasligi acikla.
```

Command Palette:

```text
intent: explain_system_slowdown
candidate_app: hn-monitor
optional_ai_app: hn-ai-lab
```

Task Engine, uygun uygulamayi onerir veya calistirma izni ister.

## HNLang App Fikri

HNLang uygulamalari task odakli olmalidir.

Ornek:

```text
app "hn-notes" {
  permission read.fs.metadata scope "/notes"
  permission write.fs.file scope "/notes"

  command "new" {
    input title
    create note title
    output "note created"
  }
}
```

HNLang app dogrudan kernel cagrisi yapmaz; runtime onu izinli syscall/task/action modeline cevirir.

## Native App Fikri

Uzun vadede HNMos native user-space uygulamalari destekleyebilir.

Native app hedefleri:

- Freestanding ABI.
- HNMos syscall wrapper.
- Capability aware runtime.
- App manifest ile baslatma.
- Panic/fault izolasyonu.

Native uygulama kernel module degildir. Kernel icinde calismaz.

## UI Modeli

Ilk UI modu:

```text
console
```

Ileride:

```text
framebuffer_panel
window
assistant_panel
task_monitor_panel
```

UI modu manifestte tanimlanir, fakat permission sistemi tarafindan sinirlanabilir.

## Kernel/User-Space Siniri

Uygulamalarin yapamayacagi islemler:

- Kernel memory write.
- Driver port I/O.
- Interrupt tablosu degistirme.
- Raw disk block write.
- Audit log silme.
- Permission self-approval.

Uygulamalarin izinli yapabilecegi islemler:

- Syscall ile public status okuma.
- Manifest kapsaminda dosya metadata okuma.
- Kullanici onayli dosya yazma.
- Task Engine'e action proposal gonderme.
- AI Runtime Interface'e izinli context request gonderme.

## App Lifecycle

Basit lifecycle:

```text
discovered
validated
permission_checked
ready
running
waiting_permission
done
failed
revoked
```

`revoked`, kullanicinin session iznini geri almasi veya policy engine'in app'i durdurmasi durumudur.

## Audit Baglantisi

App baslatma ve kritik eylemler audit log'a baglanir:

```text
app.launch
app.permission.request
app.permission.approve
app.permission.deny
app.action.execute
app.exit
app.fail
```

AI kullanan app'lerde `ai.proposal` ve `ai.context.read` kayitlari da task id ile baglanir.

## Tamamlanma Kriterleri

- HNMos uygulama modeli tanimlanmistir.
- Uygulama tipleri listelenmistir.
- Manifest zorunlulugu yazilmistir.
- Permission sistemiyle baglanti aciklanmistir.
- HNShell ve Command Palette app akislari belirtilmistir.
- HNLang ve native app fikirleri ayrilmistir.
- Kernel/user-space uygulama siniri tanimlanmistir.

## App Model Uygulama Notu

App model ilk calisabilir app discovery ve manifest okuma tasarimina indirilmelidir. Ilk hedef HNShell'in `hn app list` ve `hn app info` komutlari icin manifest indeksini okuyabilmesidir.

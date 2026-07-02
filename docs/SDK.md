# HNMos SDK

## Hedef

HNMOS-CODEX-12'nin hedefi, HNMos uzerinde calisacak kucuk sistem araclari, gorev uygulamalari ve AI destekli yardimci uygulamalar icin SDK ve uygulama modelini tasarlamaktir.

Bu SDK Windows, Linux veya macOS uygulamalari uretmek icin tasarlanmaz. Host sistem yalnizca HNMos kodunu derlemek, boot edilebilir imaj uretmek ve QEMU'da calistirmak icin kullanilir.

SDK'nin hedef ciktilari HNMos user-space icinde calisacak uygulamalardir.

## Dosya Yapisi

```text
sdk/
|-- templates/
`-- examples/

examples/
|-- hn-notes/
|-- hn-monitor/
`-- hn-ai-lab/

docs/
|-- SDK.md
`-- APP_MODEL.md
```

- `sdk/templates/`: yeni HNMos uygulamalari icin manifest ve kaynak sablonlari.
- `sdk/examples/`: SDK tasariminda referans alinacak genel ornekler.
- `examples/hn-notes/`: not, metadata ve ozetleme odakli app hedefi.
- `examples/hn-monitor/`: system status, event snapshot ve task monitor app hedefi.
- `examples/hn-ai-lab/`: AI Runtime Interface deney app hedefi.

## SDK'nin Rolu

HNMos SDK su soruya cevap verir:

```text
HNMos uzerinde guvenli ve gorev odakli bir uygulama nasil yazilir?
```

SDK, ilk asamada tam compiler veya paket yoneticisi degildir. Once uygulama sozlesmesini, manifest formatini, izin baglantisini ve HNShell calistirma modelini tanimlar.

SDK hedefleri:

- Uygulama manifest standardi.
- HNLang task app sablonu.
- User-space native app ABI hedefi.
- HNShell calistirma komutlari.
- Permission manifest entegrasyonu.
- AI Runtime Interface icin guvenli app sozlesmesi.
- QEMU icinde boot eden HNMos imajina app paketleme plani.

## HNMos Uygulama Tipleri

Ilk uygulama tipleri:

```text
system_tool
task_app
ai_assistant_app
driver_agent
dev_tool
ui_panel
```

- `system_tool`: sistem durumunu okuyan ve shell ile calisan arac.
- `task_app`: kullanici istegini task olarak isleyen uygulama.
- `ai_assistant_app`: AI Runtime Interface kullanan, izinli yardimci uygulama.
- `driver_agent`: driver event snapshot okuyan user-space agent.
- `dev_tool`: build, debug ve verify gibi gelistirme gorevlerini yoneten arac.
- `ui_panel`: framebuffer/window manager sonrasi panel olarak acilabilen uygulama.

## Ornek Uygulamalar

Hedef uygulamalar:

- `hn-notes`: not, etiket, metadata ve ozet uygulamasi.
- `hn-files`: Knowledge FS dosya ve metadata gorunumu.
- `hn-monitor`: sistem ve event monitor.
- `hn-builder`: build, verify ve hata ozetleme araci.
- `hn-ai-lab`: AI Runtime Interface deney alani.
- `hn-camera-agent`: camera event stream icin izinli agent.
- `hn-dev-studio`: HNLang, HNShell ve task monitor odakli gelistirme ortami.

## Uygulama Manifest Formati

Her uygulama bir manifest ile tanimlanir.

```json
{
  "manifest_version": 1,
  "app_id": "org.hnmos.hn-monitor",
  "name": "hn-monitor",
  "type": "system_tool",
  "entry": "main.hnl",
  "runtime": "hnlang",
  "description": "System status and event monitor for HNMos.",
  "permissions": [
    {
      "capability": "read.system_status",
      "scope": "system.summary",
      "reason": "show system health"
    }
  ],
  "shell": {
    "command": "hn app run hn-monitor",
    "aliases": [
      "hn-monitor"
    ]
  },
  "ui": {
    "mode": "console",
    "panels": [
      "task_monitor"
    ]
  },
  "ai": {
    "uses_ai": false,
    "context_policy": "none"
  }
}
```

Manifest alanlari:

- `app_id`: benzersiz uygulama kimligi.
- `name`: HNShell ve UI tarafinda gorunen ad.
- `type`: uygulama tipi.
- `entry`: uygulamanin giris dosyasi.
- `runtime`: `hnlang`, `native`, `service` veya ileride `mixed`.
- `permissions`: AI Permission Layer ile uyumlu capability listesi.
- `shell`: HNShell calistirma komutu ve alias listesi.
- `ui`: console, framebuffer veya panel hedefi.
- `ai`: AI Runtime Interface kullanim politikasi.

## Permission Sistemiyle Baglanti

SDK manifest'i CODEX-11 izin sistemiyle dogrudan baglidir.

Uygulama calistirma akisi:

```text
hn app run hn-monitor
  -> app manifest load
  -> requested permissions extract
  -> policy engine evaluate
  -> user approval if needed
  -> task/app launch
  -> audit log
```

Low risk read-only uygulamalar session policy ile calisabilir. High veya critical izin isteyen uygulamalar kullanici onayi ister.

AI kullanan uygulamalar ek kurala tabidir:

```text
AI app cannot execute system action directly.
AI app creates proposal.
Task Engine and Policy Engine decide.
User approves critical actions.
```

## HNShell Uzerinden Calistirma

HNShell, ilk uygulama baslatma yuzeyi olmalidir.

Komutlar:

```text
hn app list
hn app info hn-monitor
hn app run hn-monitor
hn app permissions hn-ai-lab
```

Uygulama baslatma asamalari:

1. Manifest bulunur.
2. App id ve runtime dogrulanir.
3. Izinler policy engine'e sorulur.
4. Gerekirse kullanici onayi alinir.
5. Runtime veya task app baslatilir.
6. Sonuc audit log'a yazilir.

## HNLang ile Uygulama Yazma Fikri

HNLang, ilk HNMos uygulamalari icin dogal adaydir. Cok kucuk sistem araclari ve task uygulamalari HNLang ile yazilabilir.

Ornek:

```text
app "hn-monitor" {
  permission read.system_status scope "system.summary"
  permission read.events.public scope "event_snapshot.public"

  command "show" {
    kernel.inspect(status)
    event.list(public)
    output console
  }
}
```

HNLang runtime, kernel'e dogrudan erismez. Task Engine, syscalls ve izinli OS servisleri uzerinden calisir.

## SDK Klasor Yapisi

Planlanan SDK yapisi:

```text
sdk/
|-- templates/
|   `-- app.manifest.json
|-- examples/
|   `-- README.md
|-- include/
|-- lib/
|-- tools/
`-- package/
```

Bu kademede sadece `templates/` ve `examples/` hazirlanir. `include/`, `lib/`, `tools/` ve `package/` HNMos ABI ve app packaging netlestikce eklenecektir.

## Ornek Uygulama Sablonu

Minimal uygulama yapisi:

```text
my-app/
|-- app.manifest.json
|-- main.hnl
`-- README.md
```

Ilk HNLang entry hedefi:

```text
app "my-app" {
  command "run" {
    output "Hello from HNMos app"
  }
}
```

## Kernel/User-Space Uygulama Siniri

Uygulamalar kernel icinde calismaz.

Kernel saglar:

- Syscall arayuzu.
- Event snapshot.
- Console/framebuffer cikis kapilari.
- FS metadata ve dosya erisim kapilari.
- Permission enforcement icin temel guvenlik sinirlari.

User-space uygulama saglar:

- Kullanici komutlari.
- Task flow.
- UI panel veya console cikti.
- AI proposal.
- Manifest ve permission scope.

Driver, kernel memory veya raw disk erisimi uygulamalara dogrudan verilmez.

## Tamamlanma Kriterleri

- `sdk/`, `sdk/templates/` ve `sdk/examples/` alanlari hazirdir.
- `examples/hn-notes/`, `examples/hn-monitor/` ve `examples/hn-ai-lab/` alanlari hazirdir.
- HNMos uygulama modeli tanimlanmistir.
- Uygulama manifest formati yazilmistir.
- Permission sistemiyle baglanti aciklanmistir.
- HNShell uzerinden uygulama calistirma plani yazilmistir.
- HNLang ile uygulama yazma fikri tanimlanmistir.
- Kernel/user-space uygulama siniri belirtilmistir.

## App Loader Uygulama Notu

SDK tasarimi ilk uygulanabilir app loader ve manifest parser taslagina indirilirken hedef:

1. `hn app list` komut taslagi.
2. `hn app info <name>` komut taslagi.
3. Manifest parser veri modeli.
4. Permission manifest baglantisi.
5. `examples/hn-monitor` icin ilk HNLang `main.hnl` taslagi.
6. App paketleme ve initramfs yerlesim plani.

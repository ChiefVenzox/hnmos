# Driver Event Layer

## Hedef

HNMOS-CODEX-09'un hedefi, donanim olaylarini kernel, shell ve AI Runtime tarafindan okunabilir standart olaylara donusturen Driver Event Layer'i tasarlamaktir.

AI donanimi dogrudan kontrol etmeyecektir. AI yalnizca izin verilen olaylari okuyabilir ve gorev onerisi uretebilir. Kritik islemler kullanici onayi ve permission/audit akisi gerektirir.

## Dosya Yapisi

```text
kernel01/
|-- drivers/
`-- events/
    |-- event.c
    |-- event.h
    |-- event_queue.c
    |-- event_queue.h
    |-- event_types.h
    `-- event_policy.h

ai/
`-- event_reader/
    |-- event_reader.c
    |-- event_reader.h
    `-- event_filter.h

shell/
`-- commands/
    `-- events/
        |-- event_list.c
        `-- event_list.h
```

Bu dosyalar CODEX-09 tasarim hedefidir. Bu kademede klasorler hazirlanir ve mimari sinirlar tanimlanir.

## Driver Event Mantigi

Driver Event Layer su donusumu yapar:

```text
hardware interrupt / driver action
  -> driver-specific raw data
  -> kernel event record
  -> kernel event queue
  -> shell / task engine / AI event reader
```

Driver'lar ham donanim verisini dogrudan shell veya AI Runtime'a vermez. Once kernel icinde tipli, sinirli, policy uygulanabilir event kaydina donusturur.

## Event Formati

Ilk event alanlari:

```text
event_id
event_type
source
timestamp_tick
severity
permission_class
payload_kind
payload_size
payload
```

Ilk event type ornekleri:

```text
keyboard.input
mouse.move
disk.read
disk.write
network.packet
gpu.framebuffer
battery.low
camera.frame
microphone.audio
```

## Kernel Event Queue Plani

Kernel event queue basit ring buffer olarak baslamalidir.

Ilk hedef:

- Sabit boyutlu event buffer.
- Monoton artan `event_id`.
- Drop sayaci.
- Event type filtreleme.
- Permission class alanini tasima.
- Interrupt context icinde minimum is yapmak.

Ilk queue API taslagi:

```text
hnm_event_push(event)
hnm_event_read_next(cursor, filter, out_event)
hnm_event_get_stats(out_stats)
```

Interrupt handler icinde agir is yapilmaz. Driver sadece kucuk event kaydi uretir, yorumlama user-space veya daha ust servislerde yapilir.

## Shell Uzerinden Event Okuma

HNShell event komutlari:

```text
hn event list
hn event list keyboard
hn event list disk
hn event stats
```

Ilk cevap ornegi:

```text
event_id=12 type=keyboard.input source=ps2 severity=info
event_id=13 type=disk.read source=ide severity=info
```

HNShell ham payload yerine guvenli event ozeti gostermelidir.

## AI Runtime'in Event Yorumlamasi

AI Runtime event queue'yu dogrudan ham kernel bellegi olarak okuyamaz.

Dogru akis:

```text
AI Runtime request
  -> event_reader
  -> event policy filter
  -> sanitized event snapshot
  -> Context Router
  -> AI Runtime Interface
```

AI event'leri kullanarak:

- Sistem durumu ozeti uretebilir.
- Gorev onerisi uretebilir.
- Riskli olaylari kullaniciya aciklayabilir.

AI sunlari yapamaz:

- Klavye/mouse/disk/network/gpu donanimini dogrudan kontrol etmek.
- Event payload icinden gizli veri cekmek.
- Kritik islemi kullanici onaysiz baslatmak.
- Driver state degistirmek.

## Guvenlik Sinirlari

Event permission class:

```text
public
system
sensitive
restricted
```

Ilk politika:

- `public`: HNShell ve AI tarafindan ozet olarak okunabilir.
- `system`: HNShell okuyabilir, AI icin Context Router filtresi gerekir.
- `sensitive`: kullanici onayi veya policy gerekir.
- `restricted`: AI tarafina verilmez.

Kritik kaynaklar:

- `keyboard.input`: karakter icerebilir, AI icin maskeleme gerekir.
- `network.packet`: payload verilmez, sadece ozet verilir.
- `camera.frame`: restricted.
- `microphone.audio`: restricted.
- `disk.write`: audit gerektirir.

## Event JSON Ornegi

```json
{
  "event_id": 42,
  "type": "keyboard.input",
  "source": "ps2.keyboard",
  "timestamp_tick": 1204,
  "severity": "info",
  "permission_class": "sensitive",
  "payload_kind": "key_event",
  "payload": {
    "key": "A",
    "pressed": true,
    "masked_for_ai": true
  }
}
```

## Event Struct Ornegi

```c
typedef struct hnm_event {
    unsigned int event_id;
    unsigned int event_type;
    unsigned int source;
    unsigned int timestamp_tick;
    unsigned int severity;
    unsigned int permission_class;
    unsigned int payload_kind;
    unsigned int payload_size;
    unsigned char payload[32];
} hnm_event;
```

## QEMU Uzerinde Ilk Test Edilebilir Event Hedefleri

QEMU'da ilk test hedefleri:

1. `keyboard.input`: PS/2 klavye IRQ1.
2. `timer.tick`: PIT IRQ0, debug event olarak.
3. `serial.input`: COM1 input varsa debug event.
4. `disk.read`: gercek disk surucusu once stub event olabilir.
5. `gpu.framebuffer`: VGA text write event'i debug seviyesinde.

Ilk uygulanabilir hedef `keyboard.input` ve `timer.tick` olmalidir.

## Tamamlanma Kriterleri

- `kernel01/events/`, `ai/event_reader/`, `shell/commands/events/` klasorleri hazirdir.
- Driver event mantigi tanimlanmistir.
- Event formati ve kernel event queue plani yazilmistir.
- HNShell uzerinden event okuma komutlari tanimlanmistir.
- AI Runtime'in sadece izinli/sanitize event snapshot okuyacagi belirtilmistir.
- Guvenlik sinirlari ve QEMU ilk test hedefleri yazilmistir.

## Event Layer Uygulama Notu

Driver Event Layer'in ilk minimal uygulamasi icin hedef:

1. `hnm_event` struct ve event type enum'unu ekle.
2. Sabit boyutlu kernel event ring buffer ekle.
3. Timer tick veya keyboard input icin ilk event push yolunu hazirla.
4. `hn event list` icin stub/snapshot cevabi tasarla.
5. AI event reader icin sadece `public` eventleri donduren filtre ekle.

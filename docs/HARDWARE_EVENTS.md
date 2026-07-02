# Hardware Events

## Hedef

Bu belge, HNMos Driver Event Layer icin ilk donanim olay sozlugunu ve event formatlarini tanimlar.

Donanim olaylari shell, task engine ve AI Runtime tarafindan okunabilir hale gelir; ancak AI donanimi dogrudan kontrol etmez.

## Olay Kategorileri

Ilk olay kategorileri:

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
timer.tick
serial.input
```

## Ortak Event Alani

Her event ortak bir baslik tasimalidir:

```text
event_id
type
source
timestamp_tick
severity
permission_class
payload_kind
payload
```

Severity:

```text
debug
info
warning
error
critical
```

Permission class:

```text
public
system
sensitive
restricted
```

## keyboard.input

Kaynak:

```text
ps2.keyboard
usb.keyboard
```

Payload:

```text
key_code
pressed
modifiers
ascii_if_safe
```

Guvenlik:

- AI'ye ham yazilan metin akisi verilmez.
- Password veya secret input alanlari maskelenir.
- Varsayilan permission class `sensitive`.

## mouse.move

Payload:

```text
dx
dy
buttons
```

Guvenlik:

- Genellikle `system`.
- AI icin yalnizca davranis ozeti verilebilir.

## disk.read

Payload:

```text
device_id
block
count
result
```

Guvenlik:

- Payload veri icerigi verilmez.
- Sadece operasyon ozeti verilir.
- Varsayilan permission class `system`.

## disk.write

Payload:

```text
device_id
block
count
result
```

Guvenlik:

- Audit gerektirir.
- AI write islemini baslatamaz.
- Varsayilan permission class `sensitive`.

## network.packet

Payload:

```text
interface_id
direction
size
protocol
summary
```

Guvenlik:

- Ham packet payload AI'ye verilmez.
- Varsayilan permission class `sensitive`.

## gpu.framebuffer

Payload:

```text
mode
width
height
buffer_state
```

Guvenlik:

- Framebuffer icerigi AI'ye ham olarak verilmez.
- Debug seviyesinde sadece durum bilgisi verilir.

## battery.low

Payload:

```text
percent
state
```

Guvenlik:

- Genellikle `public` veya `system`.
- AI gorev onerisi uretebilir: "dusuk guc modu oner".

## camera.frame

Payload:

```text
device_id
frame_count
resolution
```

Guvenlik:

- Varsayilan permission class `restricted`.
- AI ham frame okuyamaz.
- Kullanici onayi ve explicit policy olmadan acilmaz.

## microphone.audio

Payload:

```text
device_id
level
sample_rate
```

Guvenlik:

- Varsayilan permission class `restricted`.
- AI ham audio stream okuyamaz.

## timer.tick

Payload:

```text
tick
frequency
```

Guvenlik:

- QEMU testleri icin uygundur.
- Varsayilan permission class `public`.

## serial.input

Payload:

```text
byte_count
source
masked
```

Guvenlik:

- Debug amacli kullanilir.
- Ham input AI'ye verilmez.

## Shell Event Komutlari

Ilk HNShell komutlari:

```text
hn event list
hn event list keyboard
hn event list disk
hn event stats
```

Ilk command output:

```text
id=1 type=timer.tick source=pit class=public
id=2 type=keyboard.input source=ps2.keyboard class=sensitive masked=true
```

## AI Event Reader

AI event reader su filtreyi uygular:

```text
public -> allowed
system -> summarized only
sensitive -> permission required
restricted -> denied
```

AI event reader yalnizca snapshot dondurur. Kernel event queue pointer'i veya mutable event state verilmez.

## QEMU Ilk Test Plani

Ilk QEMU testleri:

1. PIT timer ile `timer.tick` event uret.
2. PS/2 klavye ile `keyboard.input` event uret.
3. VGA log yaziminda `gpu.framebuffer` debug event uret.
4. Disk surucusu hazir degilse `disk.read` stub event uret.

Basari kriteri:

```text
hn event list
```

komutu en az bir `timer.tick` veya `keyboard.input` event ozeti gosterebilmelidir.

## Tamamlanma Kriterleri

- Ilk hardware event tipleri listelenmistir.
- Ortak event alanlari tanimlanmistir.
- Permission class ve AI okuma politikalari yazilmistir.
- HNShell event komutlari belirtilmistir.
- QEMU'da test edilebilir ilk hedefler tanimlanmistir.

## Event Layer Uygulama Notu

Ilk event uygulama hedefi:

1. `timer.tick` event.
2. `keyboard.input` event.
3. Event ring buffer.
4. `hn event list` stub.
5. AI event reader public-only filtre.

# Filesystem Foundation

## Hedef

HNMOS-CODEX-10 ile Kernel 01 ilk calisan dosya sistemi temelini alir. Bu katman host dosya sistemini kullanmaz ve Linux/macOS/Windows uzerinde calisan bir uygulama degildir; QEMU'da boot eden HNMos kernel'i icinde derlenmis, read-only RAM node'lari sunar.

## Secilen Strateji

Mevcut boot image akisi tek kernel ELF dosyasini yukler:

```text
tools/image/grub.cfg
tools/image/build-iso.sh
build/hnmos-kernel01.elf
```

GRUB config henuz Multiboot module veya initrd tasimaz. ISO uretimi de yalnizca kernel ELF ve `grub.cfg` kopyalar. Bu nedenle bu batch icin en guvenli strateji:

```text
terminal
  -> VFS facade
  -> built in read-only RAM filesystem
```

Initrd veya disk dosya sistemi bu batch'e dahil degildir.

## Dosyalar

```text
kernel01/kernel/fs/
|-- vfs.*
`-- ramfs.*
```

`vfs.*`, terminal ve gelecek kernel arayuzleri icin kucuk ve sabit bir API sunar. `ramfs.*`, kernel binary icine gomulu node listesini tutar.

## Node Modeli

Ilk node yapisi:

```text
name
path
type
size
data pointer
```

Desteklenen tipler:

- `dir`
- `file`

Dinamik allocation yoktur. Tum node'lar static kernel verisidir.

## API

```text
hnm_fs_init()
hnm_fs_list(path, out_list)
hnm_fs_read(path, out_node)
hnm_fs_node_count()
```

`fs_init()` boot sirasinda PMM ve heap init'ten sonra cagrilir. Erken heap kullanmaz; heap hazir olmadan kritik allocation yapmaz.

## Mevcut Agac

```text
/
|-- system/
|   |-- version
|   `-- info
`-- readme.txt
```

Dosyalar:

- `/system/version`
- `/system/info`
- `/readme.txt`

## Terminal Komutlari

Terminal screen acikken:

```text
ls
ls /system
cat /readme.txt
cat /system/info
cat /system/version
```

Relative path'ler terminal tarafindan kok dizine normalize edilir. Ornek: `cat readme.txt`, `/readme.txt` olarak okunur.

## Grafik Arayuz

Launcher System screen artik filesystem node sayisini gosterir:

```text
filesystem nodes: 5
```

Bu, disk surucusu veya persistent storage anlamina gelmez. Yalnizca kernel icine gomulu read-only RAM node'lari sayilir.

## QEMU

Kernel ELF dogrudan QEMU ile calistirilabilir:

```sh
make kernel
./tools/qemu/run.sh kernel build/hnmos-kernel01.elf
```

Launcher acildiginda:

1. `1` ile Terminal screen'e gir.
2. `ls` yaz ve Enter'a bas.
3. `cat /readme.txt` yaz ve Enter'a bas.

Beklenen sonuc, grafik terminalde dosya listesinin ve readme iceriginin gorunmesidir.

## Sinirlar

Bu batch'te yoktur:

- Disk surucusu.
- FAT/ext veya baska gercek disk FS.
- Initrd module yukleme.
- Writable filesystem.
- File descriptor veya userspace syscall modeli.
- Metadata/Knowledge FS index.

TODO:

- GRUB Multiboot module destegi ile initrd okuma ekle.
- Disk driver hazir oldugunda gercek storage backend ekle.
- VFS node izinleri ve metadata snapshot modelini canlandir.
- `ls` ciktisina boyut ve daha zengin tip bilgisi ekle.

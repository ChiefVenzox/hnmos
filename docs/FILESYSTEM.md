# Filesystem Foundation

## Hedef

HNMOS-CODEX-10 ile Kernel 01 ilk calisan dosya sistemi temelini alir. Sistem node'lari read-only kalir; AI Studio icin `/workspace/main.hn` sinirli ve gecici bir writable RAM node'udur.

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
  -> read-only system nodes + policy-gated HNLang workspace RAM node
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
hnm_fs_write_hnlang_draft(path, data)
hnm_fs_node_count()
```

`fs_init()` boot sirasinda PMM ve heap init'ten sonra cagrilir. Erken heap kullanmaz; heap hazir olmadan kritik allocation yapmaz.

## Mevcut Agac

```text
/
|-- system/
|   |-- version
|   `-- info
|-- workspace/
|   `-- main.hn
`-- readme.txt
```

Dosyalar:

- `/system/version`
- `/system/info`
- `/readme.txt`
- `/workspace/main.hn` (policy-gated writable RAM draft)

## Terminal Komutlari

Terminal screen acikken:

```text
ls
ls /system
cat /readme.txt
cat /system/info
cat /system/version
cat /workspace/main.hn
```

Relative path'ler terminal tarafindan kok dizine normalize edilir. Ornek: `cat readme.txt`, `/readme.txt` olarak okunur.

## Grafik Arayuz

Launcher System screen artik filesystem node sayisini gosterir:

```text
filesystem nodes: 7
```

Bu, disk surucusu veya persistent storage anlamina gelmez. `/workspace/main.hn` reboot sirasinda sifirlanan RAM taslagidir; diger built-in node'lar read-only kalir.

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
- Persistent writable filesystem.
- File descriptor veya userspace syscall modeli.
- Metadata/Knowledge FS index.

TODO:

- GRUB Multiboot module destegi ile initrd okuma ekle.
- Disk driver hazir oldugunda gercek storage backend ekle.
- VFS node izinleri ve metadata snapshot modelini canlandir.
- `ls` ciktisina boyut ve daha zengin tip bilgisi ekle.

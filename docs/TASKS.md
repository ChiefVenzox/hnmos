# Kernel Task Skeleton

## Hedef

HNMOS-CODEX-11 ile Kernel 01 ilk kernel-side task/program altyapisini alir. Bu, userspace process sistemi veya ELF loader degildir. Simdilik amac, gelecekteki scheduler, program loader ve process izolasyonu icin guvenli veri yapilarini ve gorunurlugu hazirlamaktir.

## Dosyalar

```text
kernel01/kernel/task/
|-- context.*
|-- scheduler.*
`-- task.*
```

## Task Modeli

Ilk task yapisi:

```text
id
name
state
entry function
argument
stack pointer
yield count
```

Desteklenen state degerleri:

```text
READY
RUNNING
BLOCKED
STOPPED
```

Task registry static bir dizi kullanir ve dinamik allocation yapmaz. Mevcut limit `HNM_TASK_MAX = 8`.

## Scheduler Modeli

Scheduler su API'yi sunar:

```text
hnm_scheduler_init()
hnm_scheduler_add_task()
hnm_scheduler_yield()
hnm_scheduler_get_stats()
hnm_scheduler_current_task()
```

Bu batch'te `yield`, gercek CPU context switch yapmaz. Yalnizca registry icindeki `READY`/`RUNNING` state'lerini round-robin sekilde gunceller ve yield sayaclarini artirir. Timer IRQ, preemption ve stack degisimi yoktur.

## Internal Task'lar

Boot sirasinda iki kernel task kaydi eklenir:

```text
idle
ui
```

`idle`, gelecekte halt/idle davranisinin task modeli icindeki temsilidir. `ui`, su anki grafik launcher, terminal ve input poll akisini temsil eder. Bu entry function'lar henuz scheduler tarafindan cagirilmaz; mevcut main loop calismaya devam eder.

## Terminal Komutu

Terminal screen'de:

```text
tasks
```

Beklenen cikti:

```text
tasks:
scheduler: ready yields N
1 idle ready yields N
2 ui running yields N
```

Aktif task `yield` anina gore `idle` veya `ui` olabilir.

## Grafik Arayuz

Launcher System screen, task sayisini, current task adini ve scheduler yield sayisini gosterir:

```text
tasks: 2 current: ui yields: N
```

## Sinirlar

Bu batch'te yoktur:

- Preemptive multitasking.
- Timer IRQ tabanli context switch.
- Ayrilmis task stack'leri.
- Userspace.
- ELF/program loader.
- File system uzerinden program calistirma.
- Process isolation, permissions veya IPC.

## Sonraki Adimlar

Planlanan ilerlemeler:

- PIT/timer geldikten sonra scheduler tick hook'u eklemek.
- Kayitli task'lar icin ayrilmis stack hazirlamak.
- Cooperative function dispatch'i guvenli sekilde baslatmak.
- Executable loader tasarimini VFS uzerinden yapmak.
- Userspace process modeli, syscall siniri ve permission modelini eklemek.
- IPC ve process isolation'i paging/user mode hazir olduktan sonra ele almak.

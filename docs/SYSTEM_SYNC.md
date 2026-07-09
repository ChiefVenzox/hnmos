# System Sync Layer

## Hedef

System Sync Layer, Kernel 01 icinde CPU, RAM ve framebuffer/GPU gorunurlugunu tek bir OS-owned checkpoint arayuzunde toplar.

Bu katman gercek GPU command queue veya tam SMP scheduler degildir. Mevcut kademede:

- CPU icin serializing instruction kullanir.
- RAM icin locked memory barrier kullanir.
- Framebuffer hazirsa CPU-side framebuffer write barrier uygular.
- AI Runtime Interface'e sadece ozet ve sayac verir.

## Dosya Yapisi

```text
kernel01/kernel/sync/system_sync.*
kernel01/kernel/sync/sync_asm.*
```

## Assembly Helper'lar

```text
hnm_sync_cpu_serialize
hnm_sync_memory_barrier
hnm_sync_cpu_pause
hnm_sync_irq_save / hnm_sync_irq_restore
hnm_sync_lock_acquire / hnm_sync_lock_release
```

`hnm_sync_cpu_serialize`, CPU pipeline icin serializing checkpoint uretir. `hnm_sync_memory_barrier`, RAM gorunurlugu icin locked memory operation kullanir. `hnm_sync_lock_acquire`, atomic `xchg` ile kilidi alir ve bekleme dongusunde `pause` kullanir.

Snapshot kritik bolumu su sirayla calisir:

```text
EFLAGS save + cli
  -> spinlock acquire
  -> fence, counter ve snapshot update/copy
  -> spinlock release
  -> onceki EFLAGS restore
```

Boylece IRQ girisi snapshot'in bir kismini goremez ve giriste interrupt kapaliysa cikista yanlislikla acilmaz. Kilit, sync-owned alanlari tek epoch olarak korur. PMM, heap ve framebuffer kendi ortak SMP kilit protokolunu henuz sunmadigi icin bu model gelecekteki cok cekirdekli sistem icin tum alt sistemleri kapsayan atomik snapshot garantisi vermez.

## Snapshot Modeli

`struct hnm_sync_snapshot` su alanlari tasir:

```text
sequence
state_flags
cpu_fence_count
ram_fence_count
gpu_fence_count
pmm_free_pages
heap_used_bytes
framebuffer_width
framebuffer_height
framebuffer_bpp
```

Snapshot ham bellek, pointer, page table veya driver buffer acmaz.

## Framebuffer/GPU Siniri

Kernel 01 henuz gercek GPU scheduler veya command queue yonetmez. Mevcut yol yalniz CPU write ordering checkpoint'idir:

```text
framebuffer writes
  -> memory barrier
  -> gpu_fence_count
```

`gpu_fence_count`, bu CPU-side barrier'in yayinlandigini sayar. Vblank, scanout, DMA veya GPU command completion kanitlamaz. Gercek GPU driver geldikce API arkasina device-specific fence, readback, vblank veya command completion destegi eklenebilir.

## Terminal Komutlari

```text
sync
ai sync
```

`sync`, CPU/RAM ve CPU-side framebuffer checkpoint'i alir ve sayaclari yazar. `ai sync`, ayni checkpoint'i AI bridge uzerinden AI assembly planina baglar.

Bunlar kernel icindeki erken HNShell built-in komutlaridir. Host Bash veya POSIX `sh` komutlari degildir; host shell yalniz build ve testleri orkestre eder.

## AI Entegrasyonu

`ai sync` asagidaki bounded AI assembly planini uretir:

```text
observe_machine
sync_cpu
sync_ram
sync_gpu
route_context
return_result
```

Checkpoint kernel-owned ve salt gecici bir gozlem olarak hemen alinir. AI bu planla ozet durumu gorur fakat CPU, RAM veya GPU'yu dogrudan kontrol etmez; harici veya kalici eylemler permission/audit katmanlari gelene kadar gated kalir.

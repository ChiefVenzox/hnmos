#include "system_sync.h"
#include "sync_asm.h"
#include "../graphics/framebuffer.h"
#include "../log.h"
#include "../memory/heap.h"
#include "../memory/pmm.h"

static struct hnm_sync_snapshot hnm_sync_last_snapshot;
static volatile u32 hnm_sync_state_lock;
static int hnm_sync_ready;

static u32 hnm_sync_enter_critical(void)
{
    u32 irq_flags = hnm_sync_irq_save();

    hnm_sync_lock_acquire(&hnm_sync_state_lock);
    return irq_flags;
}

static void hnm_sync_leave_critical(u32 irq_flags)
{
    hnm_sync_lock_release(&hnm_sync_state_lock);
    hnm_sync_irq_restore(irq_flags);
}

static u32 hnm_sync_current_flags(void)
{
    u32 flags = HNM_SYNC_CPU_READY | HNM_SYNC_AI_VISIBLE;

    if (hnm_pmm_is_initialized() && hnm_heap_is_initialized()) {
        flags |= HNM_SYNC_RAM_READY;
    }

    if (hnm_framebuffer_is_available()) {
        flags |= HNM_SYNC_GPU_FRAMEBUFFER_READY;
    }

    return flags;
}

static void hnm_sync_cpu_ram_fence_locked(void)
{
    hnm_sync_cpu_serialize();
    hnm_sync_memory_barrier();

    hnm_sync_last_snapshot.cpu_fence_count++;
    hnm_sync_last_snapshot.ram_fence_count++;
}

static void hnm_sync_gpu_fence_locked(void)
{
    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_sync_memory_barrier();
    hnm_sync_last_snapshot.gpu_fence_count++;
}

static void hnm_sync_full_fence_locked(void)
{
    hnm_sync_cpu_ram_fence_locked();
    hnm_sync_gpu_fence_locked();
    hnm_sync_last_snapshot.sequence++;
}

static void hnm_sync_capture_locked(struct hnm_sync_snapshot *snapshot)
{
    struct hnm_pmm_stats pmm;
    struct hnm_heap_stats heap;
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();

    if (snapshot == 0) {
        return;
    }

    hnm_sync_last_snapshot.state_flags = hnm_sync_current_flags();

    if (hnm_pmm_is_initialized()) {
        hnm_pmm_get_stats(&pmm);
        hnm_sync_last_snapshot.pmm_free_pages = pmm.free_pages;
    } else {
        hnm_sync_last_snapshot.pmm_free_pages = 0;
    }

    if (hnm_heap_is_initialized()) {
        hnm_heap_get_stats(&heap);
        hnm_sync_last_snapshot.heap_used_bytes = heap.used_bytes;
    } else {
        hnm_sync_last_snapshot.heap_used_bytes = 0;
    }

    if (hnm_framebuffer_is_available()) {
        hnm_sync_last_snapshot.framebuffer_width = fb->width;
        hnm_sync_last_snapshot.framebuffer_height = fb->height;
        hnm_sync_last_snapshot.framebuffer_bpp = fb->bpp;
    } else {
        hnm_sync_last_snapshot.framebuffer_width = 0;
        hnm_sync_last_snapshot.framebuffer_height = 0;
        hnm_sync_last_snapshot.framebuffer_bpp = 0;
    }

    *snapshot = hnm_sync_last_snapshot;
}

void hnm_sync_init(void)
{
    struct hnm_sync_snapshot snapshot;
    u32 irq_flags = hnm_sync_enter_critical();

    hnm_sync_last_snapshot.sequence = 0;
    hnm_sync_last_snapshot.state_flags = hnm_sync_current_flags();
    hnm_sync_last_snapshot.cpu_fence_count = 0;
    hnm_sync_last_snapshot.ram_fence_count = 0;
    hnm_sync_last_snapshot.gpu_fence_count = 0;
    hnm_sync_last_snapshot.pmm_free_pages = 0;
    hnm_sync_last_snapshot.heap_used_bytes = 0;
    hnm_sync_last_snapshot.framebuffer_width = 0;
    hnm_sync_last_snapshot.framebuffer_height = 0;
    hnm_sync_last_snapshot.framebuffer_bpp = 0;
    hnm_sync_ready = 1;
    hnm_sync_full_fence_locked();
    hnm_sync_capture_locked(&snapshot);
    hnm_sync_leave_critical(irq_flags);

    hnm_log_write_line("sync: cpu/ram/framebuffer checkpoint layer ready.");
    hnm_log_write("sync: ");
    hnm_log_write_line(hnm_sync_summary(&snapshot));
}

void hnm_sync_cpu_ram_fence(void)
{
    u32 irq_flags = hnm_sync_enter_critical();

    hnm_sync_cpu_ram_fence_locked();
    hnm_sync_leave_critical(irq_flags);
}

void hnm_sync_gpu_fence(void)
{
    u32 irq_flags = hnm_sync_enter_critical();

    hnm_sync_gpu_fence_locked();
    hnm_sync_leave_critical(irq_flags);
}

void hnm_sync_full_fence(void)
{
    u32 irq_flags = hnm_sync_enter_critical();

    hnm_sync_full_fence_locked();
    hnm_sync_leave_critical(irq_flags);
}

void hnm_sync_capture(struct hnm_sync_snapshot *snapshot)
{
    u32 irq_flags;

    if (snapshot == 0) {
        return;
    }

    irq_flags = hnm_sync_enter_critical();
    hnm_sync_capture_locked(snapshot);
    hnm_sync_leave_critical(irq_flags);
}

void hnm_sync_checkpoint(struct hnm_sync_snapshot *snapshot)
{
    u32 irq_flags = hnm_sync_enter_critical();

    hnm_sync_full_fence_locked();
    hnm_sync_capture_locked(snapshot);
    hnm_sync_leave_critical(irq_flags);
}

const char *hnm_sync_summary(const struct hnm_sync_snapshot *snapshot)
{
    u32 flags;

    if (snapshot == 0) {
        return "sync: unavailable";
    }

    flags = snapshot->state_flags;

    if ((flags & HNM_SYNC_RAM_READY) != 0 &&
        (flags & HNM_SYNC_GPU_FRAMEBUFFER_READY) != 0) {
        return "sync: cpu serialized, ram fenced, framebuffer writes ordered";
    }

    if ((flags & HNM_SYNC_RAM_READY) != 0) {
        return "sync: cpu serialized, ram fenced, framebuffer unavailable";
    }

    if ((flags & HNM_SYNC_GPU_FRAMEBUFFER_READY) != 0) {
        return "sync: cpu serialized, ram pending, framebuffer writes ordered";
    }

    return "sync: cpu serialized, ram pending, framebuffer unavailable";
}

const char *hnm_sync_gpu_summary(const struct hnm_sync_snapshot *snapshot)
{
    if (snapshot == 0) {
        return "framebuffer sync: unavailable";
    }

    if ((snapshot->state_flags & HNM_SYNC_GPU_DRIVER_READY) != 0) {
        return "gpu sync: driver completion fence ready";
    }

    if ((snapshot->state_flags & HNM_SYNC_GPU_FRAMEBUFFER_READY) != 0) {
        return "framebuffer sync: cpu write barrier ready";
    }

    return "framebuffer sync: unavailable";
}

int hnm_sync_is_ready(void)
{
    int ready;
    u32 irq_flags = hnm_sync_enter_critical();

    ready = hnm_sync_ready;
    hnm_sync_leave_critical(irq_flags);
    return ready;
}

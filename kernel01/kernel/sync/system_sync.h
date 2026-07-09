#ifndef HNM_SYSTEM_SYNC_H
#define HNM_SYSTEM_SYNC_H

#include "../types.h"

enum hnm_sync_state_flags {
    HNM_SYNC_CPU_READY = 1 << 0,
    HNM_SYNC_RAM_READY = 1 << 1,
    HNM_SYNC_GPU_FRAMEBUFFER_READY = 1 << 2,
    HNM_SYNC_GPU_DRIVER_READY = 1 << 3,
    HNM_SYNC_AI_VISIBLE = 1 << 4
};

struct hnm_sync_snapshot {
    u32 sequence;
    u32 state_flags;
    u32 cpu_fence_count;
    u32 ram_fence_count;
    u32 gpu_fence_count;
    u32 pmm_free_pages;
    u32 heap_used_bytes;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u32 framebuffer_bpp;
};

void hnm_sync_init(void);
void hnm_sync_cpu_ram_fence(void);
void hnm_sync_gpu_fence(void);
void hnm_sync_full_fence(void);
void hnm_sync_capture(struct hnm_sync_snapshot *snapshot);
void hnm_sync_checkpoint(struct hnm_sync_snapshot *snapshot);
const char *hnm_sync_summary(const struct hnm_sync_snapshot *snapshot);
const char *hnm_sync_gpu_summary(const struct hnm_sync_snapshot *snapshot);
int hnm_sync_is_ready(void);

#endif

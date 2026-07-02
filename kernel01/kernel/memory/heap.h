#ifndef HNM_MEMORY_HEAP_H
#define HNM_MEMORY_HEAP_H

#include "../types.h"

struct hnm_heap_stats {
    u32 total_bytes;
    u32 used_bytes;
    u32 page_count;
    u32 allocation_count;
    u32 failed_allocations;
    int initialized;
    int supports_free;
};

void hnm_heap_init(void);
void *hnm_kmalloc(u32 size);
void hnm_kfree(void *ptr);
void hnm_heap_get_stats(struct hnm_heap_stats *stats);
void hnm_heap_log_stats(void);
int hnm_heap_is_initialized(void);

#endif

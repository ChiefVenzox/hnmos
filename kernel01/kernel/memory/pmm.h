#ifndef HNM_MEMORY_PMM_H
#define HNM_MEMORY_PMM_H

#include "../types.h"

enum {
    HNM_PMM_PAGE_SIZE = 4096
};

struct hnm_pmm_stats {
    u32 total_memory_kib;
    u32 usable_memory_kib;
    u32 reserved_memory_kib;
    u32 total_pages;
    u32 used_pages;
    u32 free_pages;
    u32 region_count;
    u32 usable_region_count;
    u32 reserved_region_count;
    int mmap_available;
    int initialized;
};

void hnm_pmm_init(u32 multiboot_info_addr);
void *hnm_pmm_alloc_page(void);
void hnm_pmm_free_page(void *page);
void hnm_pmm_get_stats(struct hnm_pmm_stats *stats);
void hnm_pmm_log_stats(void);
int hnm_pmm_is_initialized(void);

#endif

#include "heap.h"
#include "pmm.h"
#include "../log.h"

enum {
    HNM_HEAP_ALIGNMENT = 8,
    HNM_HEAP_MAX_ALLOCATION = HNM_PMM_PAGE_SIZE
};

static struct hnm_heap_stats hnm_heap_stats;
static u8 *hnm_heap_current_page;
static u32 hnm_heap_current_offset;

static u32 hnm_heap_align(u32 value)
{
    return (value + HNM_HEAP_ALIGNMENT - 1) & ~(HNM_HEAP_ALIGNMENT - 1);
}

static int hnm_heap_new_page(void)
{
    void *page = hnm_pmm_alloc_page();

    if (page == 0) {
        hnm_heap_stats.failed_allocations++;
        return 0;
    }

    hnm_heap_current_page = (u8 *)page;
    hnm_heap_current_offset = 0;
    hnm_heap_stats.page_count++;
    hnm_heap_stats.total_bytes += HNM_PMM_PAGE_SIZE;
    return 1;
}

void hnm_heap_init(void)
{
    hnm_heap_stats.total_bytes = 0;
    hnm_heap_stats.used_bytes = 0;
    hnm_heap_stats.page_count = 0;
    hnm_heap_stats.allocation_count = 0;
    hnm_heap_stats.failed_allocations = 0;
    hnm_heap_stats.initialized = hnm_pmm_is_initialized();
    hnm_heap_stats.supports_free = 0;
    hnm_heap_current_page = 0;
    hnm_heap_current_offset = 0;

    if (!hnm_heap_stats.initialized) {
        hnm_log_write_line("heap: PMM unavailable; heap disabled.");
        return;
    }

    hnm_log_write_line("heap: bump allocator ready.");
}

void *hnm_kmalloc(u32 size)
{
    u32 aligned_size;
    void *result;

    if (!hnm_heap_stats.initialized || size == 0) {
        return 0;
    }

    aligned_size = hnm_heap_align(size);

    if (aligned_size > HNM_HEAP_MAX_ALLOCATION) {
        hnm_heap_stats.failed_allocations++;
        return 0;
    }

    if (hnm_heap_current_page == 0 ||
        hnm_heap_current_offset + aligned_size > HNM_PMM_PAGE_SIZE) {
        if (!hnm_heap_new_page()) {
            return 0;
        }
    }

    result = hnm_heap_current_page + hnm_heap_current_offset;
    hnm_heap_current_offset += aligned_size;
    hnm_heap_stats.used_bytes += aligned_size;
    hnm_heap_stats.allocation_count++;
    return result;
}

void hnm_kfree(void *ptr)
{
    (void)ptr;
}

void hnm_heap_get_stats(struct hnm_heap_stats *stats)
{
    *stats = hnm_heap_stats;
}

void hnm_heap_log_stats(void)
{
    hnm_log_write_line("heap: stats.");
    hnm_log_write("heap pages:        ");
    hnm_log_write_hex32(hnm_heap_stats.page_count);
    hnm_log_write_line("");
    hnm_log_write("heap total bytes:  ");
    hnm_log_write_hex32(hnm_heap_stats.total_bytes);
    hnm_log_write_line("");
    hnm_log_write("heap used bytes:   ");
    hnm_log_write_hex32(hnm_heap_stats.used_bytes);
    hnm_log_write_line("");
    hnm_log_write("heap allocations:  ");
    hnm_log_write_hex32(hnm_heap_stats.allocation_count);
    hnm_log_write_line("");
}

int hnm_heap_is_initialized(void)
{
    return hnm_heap_stats.initialized;
}

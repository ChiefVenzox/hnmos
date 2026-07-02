#include "pmm.h"
#include "../log.h"

enum {
    HNM_MULTIBOOT_INFO_MEMORY = 1 << 0,
    HNM_MULTIBOOT_INFO_MMAP = 1 << 6,
    HNM_MULTIBOOT_MEMORY_AVAILABLE = 1,
    HNM_PMM_MAX_TRACKED_MEMORY = 1024 * 1024 * 1024,
    HNM_PMM_MAX_PAGES = HNM_PMM_MAX_TRACKED_MEMORY / HNM_PMM_PAGE_SIZE,
    HNM_PMM_BITMAP_BYTES = HNM_PMM_MAX_PAGES / 8,
    HNM_PMM_LOW_MEMORY_LIMIT = 0x00100000,
    HNM_PMM_KERNEL_START = 0x00100000
};

struct hnm_multiboot_info {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;
} __attribute__((packed));

struct hnm_multiboot_mmap_entry {
    u32 size;
    u32 base_addr_low;
    u32 base_addr_high;
    u32 length_low;
    u32 length_high;
    u32 type;
} __attribute__((packed));

extern char kernel_end;

static u8 hnm_pmm_bitmap[HNM_PMM_BITMAP_BYTES];
static struct hnm_pmm_stats hnm_pmm_stats;
static u32 hnm_pmm_next_search_page;

static u32 hnm_pmm_align_up(u32 value)
{
    return (value + HNM_PMM_PAGE_SIZE - 1) & ~(HNM_PMM_PAGE_SIZE - 1);
}

static u32 hnm_pmm_align_down(u32 value)
{
    return value & ~(HNM_PMM_PAGE_SIZE - 1);
}

static u32 hnm_pmm_clip_length_to_kib(u32 length_low, u32 length_high)
{
    if (length_high != 0) {
        return 0xFFFFFFFF;
    }

    return length_low / 1024;
}

static void hnm_pmm_add_kib(u32 *target, u32 amount)
{
    if (0xFFFFFFFF - *target < amount) {
        *target = 0xFFFFFFFF;
        return;
    }

    *target += amount;
}

static int hnm_pmm_page_is_used(u32 page_index)
{
    return (hnm_pmm_bitmap[page_index / 8] & (1 << (page_index % 8))) != 0;
}

static void hnm_pmm_set_page_used(u32 page_index)
{
    hnm_pmm_bitmap[page_index / 8] |= (u8)(1 << (page_index % 8));
}

static void hnm_pmm_set_page_free(u32 page_index)
{
    hnm_pmm_bitmap[page_index / 8] &= (u8)~(1 << (page_index % 8));
}

static void hnm_pmm_mark_page_free(u32 page_index)
{
    if (page_index >= hnm_pmm_stats.total_pages || !hnm_pmm_page_is_used(page_index)) {
        return;
    }

    hnm_pmm_set_page_free(page_index);
    hnm_pmm_stats.free_pages++;
    hnm_pmm_stats.used_pages--;
}

static void hnm_pmm_mark_page_used(u32 page_index)
{
    if (page_index >= hnm_pmm_stats.total_pages || hnm_pmm_page_is_used(page_index)) {
        return;
    }

    hnm_pmm_set_page_used(page_index);
    hnm_pmm_stats.free_pages--;
    hnm_pmm_stats.used_pages++;
}

static void hnm_pmm_mark_range_free(u32 base, u32 length)
{
    u32 start = hnm_pmm_align_up(base);
    u32 end;

    if (length < HNM_PMM_PAGE_SIZE) {
        return;
    }

    if (base + length < base) {
        end = HNM_PMM_MAX_TRACKED_MEMORY;
    } else {
        end = hnm_pmm_align_down(base + length);
    }

    if (end > HNM_PMM_MAX_TRACKED_MEMORY) {
        end = HNM_PMM_MAX_TRACKED_MEMORY;
    }

    for (u32 address = start; address < end; address += HNM_PMM_PAGE_SIZE) {
        hnm_pmm_mark_page_free(address / HNM_PMM_PAGE_SIZE);
    }
}

static void hnm_pmm_mark_range_used(u32 base, u32 length)
{
    u32 start = hnm_pmm_align_down(base);
    u32 end;

    if (length == 0) {
        return;
    }

    if (base + length < base) {
        end = HNM_PMM_MAX_TRACKED_MEMORY;
    } else {
        end = hnm_pmm_align_up(base + length);
    }

    if (end > HNM_PMM_MAX_TRACKED_MEMORY) {
        end = HNM_PMM_MAX_TRACKED_MEMORY;
    }

    for (u32 address = start; address < end; address += HNM_PMM_PAGE_SIZE) {
        hnm_pmm_mark_page_used(address / HNM_PMM_PAGE_SIZE);
    }
}

static void hnm_pmm_prepare_bitmap(u32 highest_address)
{
    if (highest_address > HNM_PMM_MAX_TRACKED_MEMORY) {
        highest_address = HNM_PMM_MAX_TRACKED_MEMORY;
    }

    highest_address = hnm_pmm_align_up(highest_address);
    hnm_pmm_stats.total_pages = highest_address / HNM_PMM_PAGE_SIZE;

    if (hnm_pmm_stats.total_pages > HNM_PMM_MAX_PAGES) {
        hnm_pmm_stats.total_pages = HNM_PMM_MAX_PAGES;
    }

    for (u32 i = 0; i < HNM_PMM_BITMAP_BYTES; i++) {
        hnm_pmm_bitmap[i] = 0xFF;
    }

    hnm_pmm_stats.used_pages = hnm_pmm_stats.total_pages;
    hnm_pmm_stats.free_pages = 0;
}

static u32 hnm_pmm_mmap_highest_address(const struct hnm_multiboot_info *info)
{
    u32 highest = HNM_PMM_LOW_MEMORY_LIMIT;
    u32 offset = 0;

    while (offset < info->mmap_length) {
        const struct hnm_multiboot_mmap_entry *entry =
            (const struct hnm_multiboot_mmap_entry *)(info->mmap_addr + offset);
        u32 end = HNM_PMM_MAX_TRACKED_MEMORY;

        if (entry->base_addr_high == 0 && entry->length_high == 0) {
            end = entry->base_addr_low + entry->length_low;

            if (end < entry->base_addr_low) {
                end = HNM_PMM_MAX_TRACKED_MEMORY;
            }
        }

        if (end > highest) {
            highest = end;
        }

        offset += entry->size + sizeof(entry->size);
    }

    return highest;
}

static void hnm_pmm_parse_mmap(const struct hnm_multiboot_info *info)
{
    u32 offset = 0;

    hnm_pmm_stats.mmap_available = 1;

    while (offset < info->mmap_length) {
        const struct hnm_multiboot_mmap_entry *entry =
            (const struct hnm_multiboot_mmap_entry *)(info->mmap_addr + offset);
        u32 length_kib = hnm_pmm_clip_length_to_kib(entry->length_low, entry->length_high);

        hnm_pmm_stats.region_count++;

        if (entry->type == HNM_MULTIBOOT_MEMORY_AVAILABLE) {
            hnm_pmm_stats.usable_region_count++;
            hnm_pmm_add_kib(&hnm_pmm_stats.usable_memory_kib, length_kib);

            if (entry->base_addr_high == 0) {
                hnm_pmm_mark_range_free(entry->base_addr_low, entry->length_low);
            }
        } else {
            hnm_pmm_stats.reserved_region_count++;
            hnm_pmm_add_kib(&hnm_pmm_stats.reserved_memory_kib, length_kib);
        }

        hnm_pmm_add_kib(&hnm_pmm_stats.total_memory_kib, length_kib);
        offset += entry->size + sizeof(entry->size);
    }
}

static void hnm_pmm_init_from_basic_memory(const struct hnm_multiboot_info *info)
{
    u32 total_kib = 1024;

    hnm_pmm_stats.mmap_available = 0;

    if ((info->flags & HNM_MULTIBOOT_INFO_MEMORY) != 0) {
        total_kib = info->mem_lower + info->mem_upper + 1024;
        hnm_pmm_stats.total_memory_kib = total_kib;
        hnm_pmm_stats.usable_memory_kib = info->mem_upper;
        hnm_pmm_stats.reserved_memory_kib = total_kib - info->mem_upper;
        hnm_pmm_stats.region_count = 2;
        hnm_pmm_stats.usable_region_count = 1;
        hnm_pmm_stats.reserved_region_count = 1;
        hnm_pmm_prepare_bitmap(total_kib * 1024);
        hnm_pmm_mark_range_free(HNM_PMM_LOW_MEMORY_LIMIT, info->mem_upper * 1024);
        return;
    }

    hnm_pmm_stats.total_memory_kib = total_kib;
    hnm_pmm_stats.usable_memory_kib = 0;
    hnm_pmm_stats.reserved_memory_kib = total_kib;
    hnm_pmm_prepare_bitmap(total_kib * 1024);
}

static void hnm_pmm_reserve_kernel_ranges(u32 multiboot_info_addr)
{
    u32 kernel_end_address = hnm_pmm_align_up((u32)&kernel_end);

    hnm_pmm_mark_range_used(0, HNM_PMM_LOW_MEMORY_LIMIT);
    hnm_pmm_mark_range_used(HNM_PMM_KERNEL_START, kernel_end_address - HNM_PMM_KERNEL_START);

    if (multiboot_info_addr != 0) {
        hnm_pmm_mark_range_used(multiboot_info_addr, HNM_PMM_PAGE_SIZE);
    }
}

void hnm_pmm_init(u32 multiboot_info_addr)
{
    const struct hnm_multiboot_info *info = (const struct hnm_multiboot_info *)multiboot_info_addr;
    u32 highest_address;

    hnm_pmm_stats.total_memory_kib = 0;
    hnm_pmm_stats.usable_memory_kib = 0;
    hnm_pmm_stats.reserved_memory_kib = 0;
    hnm_pmm_stats.total_pages = 0;
    hnm_pmm_stats.used_pages = 0;
    hnm_pmm_stats.free_pages = 0;
    hnm_pmm_stats.region_count = 0;
    hnm_pmm_stats.usable_region_count = 0;
    hnm_pmm_stats.reserved_region_count = 0;
    hnm_pmm_stats.mmap_available = 0;
    hnm_pmm_stats.initialized = 0;
    hnm_pmm_next_search_page = 0;

    if (multiboot_info_addr != 0 &&
        (info->flags & HNM_MULTIBOOT_INFO_MMAP) != 0 &&
        info->mmap_addr != 0 &&
        info->mmap_length != 0) {
        highest_address = hnm_pmm_mmap_highest_address(info);
        hnm_pmm_prepare_bitmap(highest_address);
        hnm_pmm_parse_mmap(info);
    } else if (multiboot_info_addr != 0) {
        hnm_pmm_init_from_basic_memory(info);
    } else {
        hnm_pmm_prepare_bitmap(HNM_PMM_LOW_MEMORY_LIMIT);
        hnm_pmm_stats.total_memory_kib = 1024;
        hnm_pmm_stats.reserved_memory_kib = 1024;
    }

    hnm_pmm_reserve_kernel_ranges(multiboot_info_addr);
    hnm_pmm_stats.initialized = 1;
    hnm_pmm_log_stats();
}

void *hnm_pmm_alloc_page(void)
{
    if (!hnm_pmm_stats.initialized || hnm_pmm_stats.free_pages == 0) {
        return 0;
    }

    for (u32 scanned = 0; scanned < hnm_pmm_stats.total_pages; scanned++) {
        u32 page = (hnm_pmm_next_search_page + scanned) % hnm_pmm_stats.total_pages;

        if (!hnm_pmm_page_is_used(page)) {
            hnm_pmm_mark_page_used(page);
            hnm_pmm_next_search_page = page + 1;
            return (void *)(page * HNM_PMM_PAGE_SIZE);
        }
    }

    return 0;
}

void hnm_pmm_free_page(void *page)
{
    u32 address = (u32)page;

    if (!hnm_pmm_stats.initialized || address == 0 || (address % HNM_PMM_PAGE_SIZE) != 0) {
        return;
    }

    hnm_pmm_mark_page_free(address / HNM_PMM_PAGE_SIZE);
}

void hnm_pmm_get_stats(struct hnm_pmm_stats *stats)
{
    *stats = hnm_pmm_stats;
}

void hnm_pmm_log_stats(void)
{
    hnm_log_write_line("memory: PMM ready.");
    hnm_log_write("memory total kib:    ");
    hnm_log_write_hex32(hnm_pmm_stats.total_memory_kib);
    hnm_log_write_line("");
    hnm_log_write("memory usable kib:   ");
    hnm_log_write_hex32(hnm_pmm_stats.usable_memory_kib);
    hnm_log_write_line("");
    hnm_log_write("memory reserved kib: ");
    hnm_log_write_hex32(hnm_pmm_stats.reserved_memory_kib);
    hnm_log_write_line("");
    hnm_log_write("memory free pages:   ");
    hnm_log_write_hex32(hnm_pmm_stats.free_pages);
    hnm_log_write_line("");
    hnm_log_write("memory mmap entries: ");
    hnm_log_write_hex32(hnm_pmm_stats.region_count);
    hnm_log_write_line("");
}

int hnm_pmm_is_initialized(void)
{
    return hnm_pmm_stats.initialized;
}

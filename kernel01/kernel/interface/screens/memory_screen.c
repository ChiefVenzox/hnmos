#include "memory_screen.h"
#include "../theme.h"
#include "../widgets/label.h"
#include "../widgets/panel.h"
#include "../widgets/window.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/primitive.h"
#include "../../log.h"
#include "../../memory/heap.h"
#include "../../memory/pmm.h"

void hnm_memory_screen_init(void)
{
}

int hnm_memory_screen_handle_event(const struct hnm_ui_event *event)
{
    (void)event;
    return 0;
}

void hnm_memory_screen_on_enter(void)
{
    hnm_log_write_line("router: memory screen entered.");
}

void hnm_memory_screen_on_exit(void)
{
    hnm_log_write_line("router: memory screen exited.");
}

static void hnm_memory_append_char(char *text, u32 *index, u32 capacity, char value)
{
    if (*index + 1 >= capacity) {
        return;
    }

    text[*index] = value;
    *index = *index + 1;
}

static void hnm_memory_append_text(char *text, u32 *index, u32 capacity, const char *value)
{
    for (u32 i = 0; value[i] != '\0'; i++) {
        hnm_memory_append_char(text, index, capacity, value[i]);
    }
}

static void hnm_memory_append_u32(char *text, u32 *index, u32 capacity, u32 value)
{
    char digits[10];
    u32 digit_count = 0;

    if (value == 0) {
        hnm_memory_append_char(text, index, capacity, '0');
        return;
    }

    while (value != 0 && digit_count < 10) {
        digits[digit_count] = (char)('0' + (value % 10));
        value /= 10;
        digit_count++;
    }

    while (digit_count > 0) {
        digit_count--;
        hnm_memory_append_char(text, index, capacity, digits[digit_count]);
    }
}

static void hnm_memory_stat_text(char *text, u32 capacity, const char *prefix, u32 value, const char *suffix)
{
    u32 index = 0;

    hnm_memory_append_text(text, &index, capacity, prefix);
    hnm_memory_append_u32(text, &index, capacity, value);
    hnm_memory_append_text(text, &index, capacity, suffix);
    text[index] = '\0';
}

static void hnm_memory_label(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = fg;
    label.background = bg;
    hnm_ui_label_render(&label);
}

void hnm_memory_screen_render(void)
{
    const struct hnm_ui_theme *theme = hnm_ui_theme_default();
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_window window;
    struct hnm_ui_panel panel;
    struct hnm_pmm_stats pmm;
    struct hnm_heap_stats heap;
    char total_text[64];
    char usable_text[64];
    char reserved_text[64];
    char pmm_text[64];
    char heap_text[64];
    char regions_text[64];
    u32 window_width = 560;
    u32 window_height = 328;
    u32 window_x;
    u32 window_y = 132;
    u32 panel_x;
    u32 panel_y;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_pmm_get_stats(&pmm);
    hnm_heap_get_stats(&heap);
    hnm_memory_stat_text(total_text, sizeof(total_text), "total memory: ", pmm.total_memory_kib, " kib");
    hnm_memory_stat_text(usable_text, sizeof(usable_text), "usable memory: ", pmm.usable_memory_kib, " kib");
    hnm_memory_stat_text(reserved_text, sizeof(reserved_text), "reserved memory: ", pmm.reserved_memory_kib, " kib");
    hnm_memory_stat_text(pmm_text, sizeof(pmm_text), "pmm free pages: ", pmm.free_pages, "");
    hnm_memory_stat_text(heap_text, sizeof(heap_text), "heap used bytes: ", heap.used_bytes, "");
    hnm_memory_stat_text(regions_text, sizeof(regions_text), "mmap entries: ", pmm.region_count, "");

    hnm_draw_fill_rect(0, 84, fb->width, fb->height - 84, theme->background);
    window_x = fb->width > window_width ? (fb->width - window_width) / 2 : 32;

    window.rect.x = window_x;
    window.rect.y = window_y;
    window.rect.width = window_width;
    window.rect.height = window_height;
    window.title = "Memory";
    window.title_background = theme->accent;
    window.title_text = theme->text;
    window.body_background = theme->panel;
    window.border = theme->border;
    hnm_ui_window_render(&window);

    panel_x = window_x + 20;
    panel_y = window_y + hnm_ui_window_title_height() + 18;
    panel.rect.x = panel_x;
    panel.rect.y = panel_y;
    panel.rect.width = window_width - 40;
    panel.rect.height = window_height - hnm_ui_window_title_height() - 44;
    panel.background = theme->panel_alt;
    panel.border = theme->border;
    hnm_ui_panel_render(&panel);

    hnm_memory_label(panel_x + 16, panel_y + 18, "physical memory manager online", theme->text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 42, total_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 66, usable_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 90, reserved_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 114, regions_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 138, pmm_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 162, heap_text, theme->muted_text, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 186, heap.supports_free ? "heap free list: ready" : "heap free list: todo bump only", theme->accent_alt, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 218, "paging and userspace memory are not ready", theme->accent_alt, theme->panel_alt);
    hnm_memory_label(panel_x + 16, panel_y + 238, "keys: 1 terminal 2 system 3 memory 4 shutdown", theme->accent_alt, theme->panel_alt);
}

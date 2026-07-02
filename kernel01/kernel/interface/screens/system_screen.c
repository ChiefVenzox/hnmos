#include "system_screen.h"
#include "../theme.h"
#include "../widgets/label.h"
#include "../widgets/panel.h"
#include "../widgets/window.h"
#include "../../fs/vfs.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/primitive.h"
#include "../../log.h"
#include "../../memory/heap.h"
#include "../../memory/pmm.h"
#include "../../task/scheduler.h"
#include "../../../drivers/mouse.h"

void hnm_system_screen_init(void)
{
}

int hnm_system_screen_handle_event(const struct hnm_ui_event *event)
{
    (void)event;
    return 0;
}

void hnm_system_screen_on_enter(void)
{
    hnm_log_write_line("router: system screen entered.");
}

void hnm_system_screen_on_exit(void)
{
    hnm_log_write_line("router: system screen exited.");
}

static void hnm_system_append_char(char *text, u32 *index, u32 capacity, char value)
{
    if (*index + 1 >= capacity) {
        return;
    }

    text[*index] = value;
    *index = *index + 1;
}

static void hnm_system_append_text(char *text, u32 *index, u32 capacity, const char *value)
{
    for (u32 i = 0; value[i] != '\0'; i++) {
        hnm_system_append_char(text, index, capacity, value[i]);
    }
}

static void hnm_system_append_u32(char *text, u32 *index, u32 capacity, u32 value)
{
    char digits[10];
    u32 digit_count = 0;

    if (value == 0) {
        hnm_system_append_char(text, index, capacity, '0');
        return;
    }

    while (value != 0 && digit_count < 10) {
        digits[digit_count] = (char)('0' + (value % 10));
        value /= 10;
        digit_count++;
    }

    while (digit_count > 0) {
        digit_count--;
        hnm_system_append_char(text, index, capacity, digits[digit_count]);
    }
}

static void hnm_system_label(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = fg;
    label.background = bg;
    hnm_ui_label_render(&label);
}

static void hnm_system_resolution_text(char *text, u32 capacity)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 index = 0;

    hnm_system_append_text(text, &index, capacity, "framebuffer: ");
    hnm_system_append_u32(text, &index, capacity, fb->width);
    hnm_system_append_text(text, &index, capacity, "x");
    hnm_system_append_u32(text, &index, capacity, fb->height);
    hnm_system_append_text(text, &index, capacity, "x");
    hnm_system_append_u32(text, &index, capacity, fb->bpp);
    text[index] = '\0';
}

static void hnm_system_stat_text(char *text, u32 capacity, const char *prefix, u32 value, const char *suffix)
{
    u32 index = 0;

    hnm_system_append_text(text, &index, capacity, prefix);
    hnm_system_append_u32(text, &index, capacity, value);
    hnm_system_append_text(text, &index, capacity, suffix);
    text[index] = '\0';
}

static void hnm_system_task_text(char *text, u32 capacity)
{
    struct hnm_scheduler_stats stats;
    const struct hnm_task *current;
    u32 index = 0;

    hnm_scheduler_get_stats(&stats);
    current = hnm_scheduler_current_task();

    hnm_system_append_text(text, &index, capacity, "tasks: ");
    hnm_system_append_u32(text, &index, capacity, stats.task_count);
    hnm_system_append_text(text, &index, capacity, " current: ");
    hnm_system_append_text(text, &index, capacity, current != 0 ? current->name : "none");
    hnm_system_append_text(text, &index, capacity, " yields: ");
    hnm_system_append_u32(text, &index, capacity, stats.yield_count);
    text[index] = '\0';
}

void hnm_system_screen_render(void)
{
    const struct hnm_ui_theme *theme = hnm_ui_theme_default();
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_window window;
    struct hnm_ui_panel panel;
    struct hnm_pmm_stats pmm;
    struct hnm_heap_stats heap;
    char resolution[48];
    char memory_text[64];
    char heap_text[64];
    char fs_text[64];
    char task_text[80];
    u32 window_width = 560;
    u32 window_height = 328;
    u32 window_x;
    u32 window_y = 132;
    u32 panel_x;
    u32 panel_y;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_draw_fill_rect(0, 84, fb->width, fb->height - 84, theme->background);
    window_x = fb->width > window_width ? (fb->width - window_width) / 2 : 32;

    window.rect.x = window_x;
    window.rect.y = window_y;
    window.rect.width = window_width;
    window.rect.height = window_height;
    window.title = "System";
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

    hnm_system_resolution_text(resolution, sizeof(resolution));
    hnm_pmm_get_stats(&pmm);
    hnm_heap_get_stats(&heap);
    hnm_system_stat_text(memory_text, sizeof(memory_text), "memory usable: ", pmm.usable_memory_kib, " kib");
    hnm_system_stat_text(heap_text, sizeof(heap_text), "heap pages: ", heap.page_count, "");
    hnm_system_stat_text(fs_text, sizeof(fs_text), "filesystem nodes: ", hnm_fs_node_count(), "");
    hnm_system_task_text(task_text, sizeof(task_text));
    hnm_system_label(panel_x + 16, panel_y + 18, "kernel: HNMos Kernel 01", theme->text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 42, resolution, theme->text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 66, memory_text, theme->muted_text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 90, heap_text, theme->muted_text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 114, "input: keyboard event queue ready", theme->muted_text, theme->panel_alt);
    hnm_system_label(
        panel_x + 16,
        panel_y + 138,
        hnm_mouse_is_ready() ? "mouse: ps2 ready" : "mouse: unavailable",
        theme->muted_text,
        theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 162, fs_text, theme->muted_text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 186, task_text, theme->muted_text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 210, "tick: no timer irq yet", theme->muted_text, theme->panel_alt);
    hnm_system_label(panel_x + 16, panel_y + 240, "keys: 1 terminal 2 system 3 memory 4 shutdown", theme->accent_alt, theme->panel_alt);
}

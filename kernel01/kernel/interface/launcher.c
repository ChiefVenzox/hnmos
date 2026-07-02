#include "launcher.h"
#include "router.h"
#include "theme.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/panel.h"
#include "../graphics/framebuffer.h"
#include "../graphics/primitive.h"
#include "../log.h"
#include "../panic.h"

enum {
    HNM_LAUNCHER_ITEM_TERMINAL = 0,
    HNM_LAUNCHER_ITEM_SYSTEM = 1,
    HNM_LAUNCHER_ITEM_MEMORY = 2,
    HNM_LAUNCHER_ITEM_AI = 3,
    HNM_LAUNCHER_ITEM_SHUTDOWN = 4,
    HNM_LAUNCHER_ITEM_COUNT = 5,
    HNM_LAUNCHER_TOP_HEIGHT = 84
};

static const struct hnm_ui_theme *hnm_launcher_theme;
static struct hnm_ui_button hnm_launcher_items[HNM_LAUNCHER_ITEM_COUNT];
static int hnm_launcher_pressed_index;
static int hnm_launcher_ready;

static void hnm_launcher_label(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = fg;
    label.background = bg;
    hnm_ui_label_render(&label);
}

static void hnm_launcher_render_top_bar(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();

    hnm_draw_fill_rect(0, 0, fb->width, HNM_LAUNCHER_TOP_HEIGHT, hnm_launcher_theme->top_bar);
    hnm_draw_fill_rect(0, HNM_LAUNCHER_TOP_HEIGHT - 2, fb->width, 2, hnm_launcher_theme->border);
    hnm_launcher_label(32, 18, "HNMos", hnm_launcher_theme->text, hnm_launcher_theme->top_bar);
    hnm_launcher_label(32, 42, "Current screen:", hnm_launcher_theme->muted_text, hnm_launcher_theme->top_bar);
    hnm_launcher_label(160, 42, ui_current_screen_name(), hnm_launcher_theme->text, hnm_launcher_theme->top_bar);
    hnm_launcher_label(256, 42, "1 Terminal 2 System 3 Memory 5 AI 4 Shutdown F1 Help", hnm_launcher_theme->accent_alt, hnm_launcher_theme->top_bar);
    hnm_launcher_label(fb->width > 248 ? fb->width - 248 : 32, 18, "tick: no timer yet", hnm_launcher_theme->muted_text, hnm_launcher_theme->top_bar);
}

static void hnm_launcher_set_item(
    u32 index,
    u32 x,
    u32 y,
    u32 width,
    u32 height,
    const char *text)
{
    hnm_launcher_items[index].rect.x = x;
    hnm_launcher_items[index].rect.y = y;
    hnm_launcher_items[index].rect.width = width;
    hnm_launcher_items[index].rect.height = height;
    hnm_launcher_items[index].text = text;
    hnm_launcher_items[index].normal_background = hnm_launcher_theme->button;
    hnm_launcher_items[index].hover_background = hnm_launcher_theme->button_hover;
    hnm_launcher_items[index].pressed_background = hnm_launcher_theme->button_pressed;
    hnm_launcher_items[index].border = hnm_launcher_theme->border;
    hnm_launcher_items[index].text_color = hnm_launcher_theme->button_text;
    hnm_launcher_items[index].hovered = 0;
    hnm_launcher_items[index].pressed = 0;
}

static void hnm_launcher_render_items(void)
{
    for (u32 i = 0; i < HNM_LAUNCHER_ITEM_COUNT; i++) {
        hnm_ui_button_render(&hnm_launcher_items[i]);
    }
}

static void hnm_launcher_render_home(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_panel panel;
    u32 panel_width = 600;
    u32 panel_height = 404;
    u32 panel_x = fb->width > panel_width ? (fb->width - panel_width) / 2 : 24;
    u32 panel_y = 142;
    u32 button_width = 220;
    u32 button_height = 56;
    u32 left_x = panel_x + 64;
    u32 right_x = panel_x + panel_width - 64 - button_width;
    u32 center_x = panel_x + ((panel_width - button_width) / 2);
    u32 top_y = panel_y + 120;
    u32 middle_y = top_y + 76;
    u32 bottom_y = middle_y + 76;

    hnm_draw_fill_rect(0, 0, fb->width, fb->height, hnm_launcher_theme->background);
    hnm_launcher_render_top_bar();

    panel.rect.x = panel_x;
    panel.rect.y = panel_y;
    panel.rect.width = panel_width;
    panel.rect.height = panel_height;
    panel.background = hnm_launcher_theme->panel;
    panel.border = hnm_launcher_theme->border;
    hnm_ui_panel_render(&panel);

    hnm_launcher_label(panel_x + 40, panel_y + 32, "Welcome to HNMos", hnm_launcher_theme->text, hnm_launcher_theme->panel);
    hnm_launcher_label(panel_x + 40, panel_y + 58, "first kernel launcher screen", hnm_launcher_theme->muted_text, hnm_launcher_theme->panel);
    hnm_launcher_label(panel_x + 40, panel_y + 84, "select an internal screen", hnm_launcher_theme->accent_alt, hnm_launcher_theme->panel);

    hnm_launcher_set_item(HNM_LAUNCHER_ITEM_TERMINAL, left_x, top_y, button_width, button_height, "1 Terminal");
    hnm_launcher_set_item(HNM_LAUNCHER_ITEM_SYSTEM, right_x, top_y, button_width, button_height, "2 System");
    hnm_launcher_set_item(HNM_LAUNCHER_ITEM_MEMORY, left_x, middle_y, button_width, button_height, "3 Memory");
    hnm_launcher_set_item(HNM_LAUNCHER_ITEM_AI, right_x, middle_y, button_width, button_height, "5 AI Panel");
    hnm_launcher_set_item(HNM_LAUNCHER_ITEM_SHUTDOWN, center_x, bottom_y, button_width, button_height, "4 Shutdown");
    hnm_launcher_render_items();
    hnm_launcher_label(panel_x + 40, panel_y + panel_height - 40, "terminal and serial debug stay available", hnm_launcher_theme->muted_text, hnm_launcher_theme->panel);
}

static void hnm_launcher_open(enum hnm_screen_id screen)
{
    hnm_launcher_pressed_index = -1;
    ui_set_screen(screen);

    if (screen == SCREEN_TERMINAL) {
        hnm_log_write_line("launcher: terminal screen opened.");
    } else if (screen == SCREEN_SYSTEM) {
        hnm_log_write_line("launcher: system screen opened.");
    } else if (screen == SCREEN_MEMORY) {
        hnm_log_write_line("launcher: memory screen opened.");
    } else if (screen == SCREEN_AI_PANEL_RESERVED) {
        hnm_log_write_line("launcher: AI panel opened.");
    }
}

static void hnm_launcher_shutdown(void)
{
    hnm_log_write_line("launcher: shutdown selected.");
    hnm_kernel_halt();
}

static void hnm_launcher_activate_item(u32 index)
{
    if (index == HNM_LAUNCHER_ITEM_TERMINAL) {
        hnm_launcher_open(SCREEN_TERMINAL);
    } else if (index == HNM_LAUNCHER_ITEM_SYSTEM) {
        hnm_launcher_open(SCREEN_SYSTEM);
    } else if (index == HNM_LAUNCHER_ITEM_MEMORY) {
        hnm_launcher_open(SCREEN_MEMORY);
    } else if (index == HNM_LAUNCHER_ITEM_AI) {
        hnm_launcher_open(SCREEN_AI_PANEL_RESERVED);
    } else if (index == HNM_LAUNCHER_ITEM_SHUTDOWN) {
        hnm_launcher_shutdown();
    }
}

static int hnm_launcher_handle_shortcut(char character)
{
    if (character == '1') {
        hnm_launcher_open(SCREEN_TERMINAL);
        return 1;
    }

    if (character == '2') {
        hnm_launcher_open(SCREEN_SYSTEM);
        return 1;
    }

    if (character == '3') {
        hnm_launcher_open(SCREEN_MEMORY);
        return 1;
    }

    if (character == '4') {
        hnm_launcher_shutdown();
        return 1;
    }

    if (character == '5') {
        hnm_launcher_open(SCREEN_AI_PANEL_RESERVED);
        return 1;
    }

    return 0;
}

static int hnm_launcher_update_hover(u32 x, u32 y)
{
    int changed = 0;

    for (u32 i = 0; i < HNM_LAUNCHER_ITEM_COUNT; i++) {
        int hovered = hnm_ui_button_hit_test(&hnm_launcher_items[i], x, y);

        if (hnm_launcher_items[i].hovered != hovered) {
            hnm_launcher_items[i].hovered = hovered;
            changed = 1;
        }
    }

    if (changed) {
        hnm_launcher_render_items();
    }

    return changed;
}

static int hnm_launcher_mouse_down(u32 x, u32 y)
{
    int changed = 0;

    hnm_launcher_pressed_index = -1;
    hnm_launcher_update_hover(x, y);

    for (u32 i = 0; i < HNM_LAUNCHER_ITEM_COUNT; i++) {
        int pressed = hnm_launcher_items[i].hovered;

        if (pressed) {
            hnm_launcher_pressed_index = (int)i;
        }

        if (hnm_launcher_items[i].pressed != pressed) {
            hnm_launcher_items[i].pressed = pressed;
            changed = 1;
        }
    }

    if (changed) {
        hnm_launcher_render_items();
    }

    return hnm_launcher_pressed_index >= 0;
}

static int hnm_launcher_mouse_up(u32 x, u32 y)
{
    int pressed_index = hnm_launcher_pressed_index;
    int consumed = 0;

    hnm_launcher_pressed_index = -1;
    hnm_launcher_update_hover(x, y);

    for (u32 i = 0; i < HNM_LAUNCHER_ITEM_COUNT; i++) {
        hnm_launcher_items[i].pressed = 0;
    }

    if (pressed_index >= 0 &&
        hnm_ui_button_hit_test(&hnm_launcher_items[pressed_index], x, y)) {
        hnm_launcher_activate_item((u32)pressed_index);
        consumed = 1;
    } else {
        hnm_launcher_render_items();
    }

    return consumed;
}

void hnm_launcher_init(void)
{
    if (!hnm_framebuffer_is_available()) {
        hnm_launcher_ready = 0;
        return;
    }

    hnm_launcher_theme = hnm_ui_theme_default();
    hnm_launcher_pressed_index = -1;
    hnm_launcher_ready = 1;
    hnm_log_write_line("launcher: ready.");
}

void hnm_launcher_render(void)
{
    if (!hnm_launcher_ready) {
        return;
    }

    hnm_launcher_render_home();
}

void hnm_launcher_on_enter(void)
{
    hnm_launcher_pressed_index = -1;

    for (u32 i = 0; i < HNM_LAUNCHER_ITEM_COUNT; i++) {
        hnm_launcher_items[i].hovered = 0;
        hnm_launcher_items[i].pressed = 0;
    }
}

void hnm_launcher_on_exit(void)
{
    hnm_launcher_pressed_index = -1;
}

int hnm_launcher_handle_event(const struct hnm_ui_event *event)
{
    if (!hnm_launcher_ready) {
        return 0;
    }

    if (event->type == HNM_UI_EVENT_KEY_CHAR) {
        return hnm_launcher_handle_shortcut(event->character);
    }

    if (event->type == HNM_UI_EVENT_MOUSE_MOVE) {
        hnm_launcher_update_hover(event->x, event->y);
        return 0;
    }

    if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_DOWN &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        return hnm_launcher_mouse_down(event->x, event->y);
    }

    if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_UP &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        return hnm_launcher_mouse_up(event->x, event->y);
    }

    return 0;
}

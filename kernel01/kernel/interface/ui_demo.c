#include "ui_demo.h"
#include "theme.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/panel.h"
#include "widgets/window.h"
#include "../graphics/font.h"
#include "../graphics/framebuffer.h"
#include "../graphics/primitive.h"
#include "../log.h"

enum {
    HNM_UI_DEMO_BUTTON_COUNT = 3
};

static struct hnm_ui_button hnm_ui_demo_buttons[HNM_UI_DEMO_BUTTON_COUNT];
static const struct hnm_ui_theme *hnm_ui_demo_theme;
static u32 hnm_ui_demo_window_x;
static u32 hnm_ui_demo_window_y;
static u32 hnm_ui_demo_status_x;
static u32 hnm_ui_demo_status_y;
static int hnm_ui_demo_initialized;
static const char *hnm_ui_demo_status_text;

static void hnm_ui_demo_draw_label(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = fg;
    label.background = bg;
    hnm_ui_label_render(&label);
}

static void hnm_ui_demo_render_status(void)
{
    hnm_draw_fill_rect(hnm_ui_demo_status_x, hnm_ui_demo_status_y, 304, 18, hnm_ui_demo_theme->panel);
    hnm_ui_demo_draw_label(
        hnm_ui_demo_status_x + 8,
        hnm_ui_demo_status_y + 5,
        hnm_ui_demo_status_text,
        hnm_ui_demo_theme->accent_alt,
        hnm_ui_demo_theme->panel);
}

static void hnm_ui_demo_render_buttons(void)
{
    for (u32 i = 0; i < HNM_UI_DEMO_BUTTON_COUNT; i++) {
        hnm_ui_button_render(&hnm_ui_demo_buttons[i]);
    }
}

static void hnm_ui_demo_set_button(
    u32 index,
    u32 x,
    u32 y,
    u32 width,
    const char *text)
{
    hnm_ui_demo_buttons[index].rect.x = x;
    hnm_ui_demo_buttons[index].rect.y = y;
    hnm_ui_demo_buttons[index].rect.width = width;
    hnm_ui_demo_buttons[index].rect.height = 28;
    hnm_ui_demo_buttons[index].text = text;
    hnm_ui_demo_buttons[index].normal_background = hnm_ui_demo_theme->button;
    hnm_ui_demo_buttons[index].hover_background = hnm_ui_demo_theme->button_hover;
    hnm_ui_demo_buttons[index].pressed_background = hnm_ui_demo_theme->button_pressed;
    hnm_ui_demo_buttons[index].border = hnm_ui_demo_theme->border;
    hnm_ui_demo_buttons[index].text_color = hnm_ui_demo_theme->button_text;
    hnm_ui_demo_buttons[index].hovered = 0;
    hnm_ui_demo_buttons[index].pressed = 0;
}

static void hnm_ui_demo_render_static(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_window window;
    struct hnm_ui_panel panel;
    u32 top_y = 84;
    u32 window_width = 352;
    u32 window_height = 236;
    u32 panel_x;
    u32 panel_y;

    hnm_draw_fill_rect(0, top_y, fb->width, 28, hnm_ui_demo_theme->top_bar);
    hnm_draw_fill_rect(0, top_y + 27, fb->width, 1, hnm_ui_demo_theme->accent);
    hnm_ui_demo_draw_label(32, top_y + 10, "HNMos interface preview", hnm_ui_demo_theme->text, hnm_ui_demo_theme->top_bar);
    hnm_ui_demo_draw_label(288, top_y + 10, "kernel widgets online", hnm_ui_demo_theme->muted_text, hnm_ui_demo_theme->top_bar);

    hnm_ui_demo_window_x = fb->width > window_width + 32 ? fb->width - window_width - 32 : 32;
    hnm_ui_demo_window_y = 124;
    window.rect.x = hnm_ui_demo_window_x;
    window.rect.y = hnm_ui_demo_window_y;
    window.rect.width = window_width;
    window.rect.height = window_height;
    window.title = "Control Surface";
    window.title_background = hnm_ui_demo_theme->accent;
    window.title_text = hnm_ui_demo_theme->text;
    window.body_background = hnm_ui_demo_theme->panel;
    window.border = hnm_ui_demo_theme->border;
    hnm_ui_window_render(&window);

    panel_x = hnm_ui_demo_window_x + 16;
    panel_y = hnm_ui_demo_window_y + hnm_ui_window_title_height() + 16;
    panel.rect.x = panel_x;
    panel.rect.y = panel_y;
    panel.rect.width = window_width - 32;
    panel.rect.height = 108;
    panel.background = hnm_ui_demo_theme->panel_alt;
    panel.border = hnm_ui_demo_theme->border;
    hnm_ui_panel_render(&panel);

    hnm_ui_demo_draw_label(panel_x + 12, panel_y + 14, "UI primitives online", hnm_ui_demo_theme->text, hnm_ui_demo_theme->panel_alt);
    hnm_ui_demo_draw_label(panel_x + 12, panel_y + 34, "events drive hover state", hnm_ui_demo_theme->muted_text, hnm_ui_demo_theme->panel_alt);
    hnm_ui_demo_draw_label(panel_x + 12, panel_y + 54, "terminal stays available", hnm_ui_demo_theme->muted_text, hnm_ui_demo_theme->panel_alt);

    hnm_ui_demo_set_button(0, panel_x + 12, panel_y + 72, 88, "Start");
    hnm_ui_demo_set_button(1, panel_x + 116, panel_y + 72, 88, "Tasks");
    hnm_ui_demo_set_button(2, panel_x + 220, panel_y + 72, 88, "About");
    hnm_ui_demo_render_buttons();

    hnm_ui_demo_status_x = panel_x;
    hnm_ui_demo_status_y = hnm_ui_demo_window_y + window_height - 30;
    hnm_ui_demo_status_text = "button: none";
    hnm_ui_demo_render_status();
}

static const char *hnm_ui_demo_button_status(u32 index)
{
    if (index == 0) {
        return "button: start";
    }

    if (index == 1) {
        return "button: tasks";
    }

    return "button: about";
}

static void hnm_ui_demo_update_hover(u32 x, u32 y)
{
    int changed = 0;

    for (u32 i = 0; i < HNM_UI_DEMO_BUTTON_COUNT; i++) {
        int hovered = hnm_ui_button_hit_test(&hnm_ui_demo_buttons[i], x, y);

        if (hnm_ui_demo_buttons[i].hovered != hovered) {
            hnm_ui_demo_buttons[i].hovered = hovered;
            changed = 1;
        }
    }

    if (changed) {
        hnm_ui_demo_render_buttons();
    }
}

static void hnm_ui_demo_clear_pressed(void)
{
    int changed = 0;

    for (u32 i = 0; i < HNM_UI_DEMO_BUTTON_COUNT; i++) {
        if (hnm_ui_demo_buttons[i].pressed) {
            hnm_ui_demo_buttons[i].pressed = 0;
            changed = 1;
        }
    }

    if (changed) {
        hnm_ui_demo_render_buttons();
    }
}

static void hnm_ui_demo_press_hovered(void)
{
    int changed = 0;

    for (u32 i = 0; i < HNM_UI_DEMO_BUTTON_COUNT; i++) {
        int pressed = hnm_ui_demo_buttons[i].hovered;

        if (hnm_ui_demo_buttons[i].pressed != pressed) {
            hnm_ui_demo_buttons[i].pressed = pressed;
            changed = 1;
        }

        if (pressed) {
            hnm_ui_demo_status_text = hnm_ui_demo_button_status(i);
        }
    }

    if (changed) {
        hnm_ui_demo_render_buttons();
        hnm_ui_demo_render_status();
    }
}

void hnm_ui_demo_init(void)
{
    if (!hnm_framebuffer_is_available()) {
        hnm_ui_demo_initialized = 0;
        return;
    }

    hnm_ui_demo_theme = hnm_ui_theme_default();
    hnm_ui_demo_initialized = 1;
    hnm_ui_demo_render_static();
    hnm_log_write_line("ui demo: primitives rendered.");
}

void hnm_ui_demo_handle_event(const struct hnm_ui_event *event)
{
    if (!hnm_ui_demo_initialized) {
        return;
    }

    if (event->type == HNM_UI_EVENT_MOUSE_MOVE) {
        hnm_ui_demo_update_hover(event->x, event->y);
    } else if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_DOWN &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        hnm_ui_demo_update_hover(event->x, event->y);
        hnm_ui_demo_press_hovered();
    } else if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_UP &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        hnm_ui_demo_update_hover(event->x, event->y);
        hnm_ui_demo_clear_pressed();
    }
}

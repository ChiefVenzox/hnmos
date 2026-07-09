#include "router.h"
#include "launcher.h"
#include "screens/memory_screen.h"
#include "screens/system_screen.h"
#include "screens/terminal_screen.h"
#include "theme.h"
#include "widgets/label.h"
#include "widgets/panel.h"
#include "../ai/ai_bridge.h"
#include "../graphics/framebuffer.h"
#include "../graphics/primitive.h"
#include "../log.h"
#include "../panic.h"

enum {
    HNM_ROUTER_TOP_HEIGHT = 84,
    HNM_ROUTER_HELP_HEIGHT = 84,
    HNM_ROUTER_HELP_MARGIN = 24
};

static enum hnm_screen_id hnm_router_current_screen = SCREEN_LAUNCHER;
static int hnm_router_ready;

static void hnm_router_reserved_init(void)
{
}

static void hnm_router_reserved_render(void)
{
    const struct hnm_ui_theme *theme = hnm_ui_theme_default();
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_panel panel;
    struct hnm_ui_label label;
    u32 panel_width = 520;
    u32 panel_height = 296;
    u32 panel_x;
    u32 panel_y = 152;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_draw_fill_rect(0, 84, fb->width, fb->height - 84, theme->background);
    panel_x = fb->width > panel_width ? (fb->width - panel_width) / 2 : 24;

    panel.rect.x = panel_x;
    panel.rect.y = panel_y;
    panel.rect.width = panel_width;
    panel.rect.height = panel_height;
    panel.background = theme->panel;
    panel.border = theme->border;
    hnm_ui_panel_render(&panel);

    label.x = panel_x + 24;
    label.y = panel_y + 32;
    label.text = "HNMos AI Panel";
    label.foreground = theme->text;
    label.background = theme->panel;
    hnm_ui_label_render(&label);

    label.y = panel_y + 58;
    label.text = "status: Stub provider active";
    label.foreground = theme->muted_text;
    hnm_ui_label_render(&label);

    label.y = panel_y + 82;
    label.text = "Real AI runtime not enabled yet";
    label.foreground = theme->accent_alt;
    hnm_ui_label_render(&label);

    label.y = panel_y + 112;
    label.text = "provider:";
    label.foreground = theme->muted_text;
    hnm_ui_label_render(&label);

    label.x = panel_x + 112;
    label.text = hnm_ai_bridge_provider_name();
    label.foreground = theme->text;
    hnm_ui_label_render(&label);

    label.x = panel_x + 24;
    label.y = panel_y + 136;
    label.text = "provider type:";
    label.foreground = theme->muted_text;
    hnm_ui_label_render(&label);

    label.x = panel_x + 144;
    label.text = hnm_ai_bridge_provider_type_name();
    label.foreground = theme->text;
    hnm_ui_label_render(&label);

    label.x = panel_x + 24;
    label.y = panel_y + 166;
    label.text = hnm_ai_bridge_machine_summary();
    label.foreground = theme->muted_text;
    hnm_ui_label_render(&label);

    label.y = panel_y + 190;
    label.text = hnm_ai_bridge_sync_summary();
    label.foreground = theme->muted_text;
    hnm_ui_label_render(&label);

    label.y = panel_y + 214;
    label.text = "ai asm: observe -> route -> draft -> permission";
    label.foreground = theme->accent_alt;
    hnm_ui_label_render(&label);

    label.y = panel_y + 244;
    label.text = "ESC returns to Launcher";
    label.foreground = theme->accent_alt;
    hnm_ui_label_render(&label);
}

static int hnm_router_reserved_handle_event(const struct hnm_ui_event *event)
{
    (void)event;
    return 0;
}

static void hnm_router_reserved_on_enter(void)
{
    hnm_log_write_line("router: AI panel opened.");
}

static void hnm_router_reserved_on_exit(void)
{
}

static const struct hnm_screen hnm_router_screens[] = {
    {
        SCREEN_LAUNCHER,
        "Launcher",
        hnm_launcher_init,
        hnm_launcher_render,
        hnm_launcher_handle_event,
        hnm_launcher_on_enter,
        hnm_launcher_on_exit
    },
    {
        SCREEN_TERMINAL,
        "Terminal",
        hnm_terminal_screen_init,
        hnm_terminal_screen_render,
        hnm_terminal_screen_handle_event,
        hnm_terminal_screen_on_enter,
        hnm_terminal_screen_on_exit
    },
    {
        SCREEN_SYSTEM,
        "System",
        hnm_system_screen_init,
        hnm_system_screen_render,
        hnm_system_screen_handle_event,
        hnm_system_screen_on_enter,
        hnm_system_screen_on_exit
    },
    {
        SCREEN_MEMORY,
        "Memory",
        hnm_memory_screen_init,
        hnm_memory_screen_render,
        hnm_memory_screen_handle_event,
        hnm_memory_screen_on_enter,
        hnm_memory_screen_on_exit
    },
    {
        SCREEN_AI_PANEL_RESERVED,
        "AI Panel",
        hnm_router_reserved_init,
        hnm_router_reserved_render,
        hnm_router_reserved_handle_event,
        hnm_router_reserved_on_enter,
        hnm_router_reserved_on_exit
    }
};

static const struct hnm_screen *hnm_router_find_screen(enum hnm_screen_id screen_id)
{
    for (u32 i = 0; i < sizeof(hnm_router_screens) / sizeof(hnm_router_screens[0]); i++) {
        if (hnm_router_screens[i].id == screen_id) {
            return &hnm_router_screens[i];
        }
    }

    return 0;
}

static const struct hnm_screen *hnm_router_current(void)
{
    return hnm_router_find_screen(hnm_router_current_screen);
}

static void hnm_router_label(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = fg;
    label.background = bg;
    hnm_ui_label_render(&label);
}

static void hnm_router_render_top_bar(void)
{
    const struct hnm_ui_theme *theme = hnm_ui_theme_default();
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_draw_fill_rect(0, 0, fb->width, HNM_ROUTER_TOP_HEIGHT, theme->top_bar);
    hnm_draw_fill_rect(0, HNM_ROUTER_TOP_HEIGHT - 2, fb->width, 2, theme->border);
    hnm_router_label(32, 18, "HNMos", theme->text, theme->top_bar);
    hnm_router_label(32, 42, "Current screen:", theme->muted_text, theme->top_bar);
    hnm_router_label(160, 42, ui_current_screen_name(), theme->text, theme->top_bar);
    hnm_router_label(256, 42, "1 Terminal 2 System 3 Memory 5 AI ESC Launcher F1 Help", theme->accent_alt, theme->top_bar);
    hnm_router_label(fb->width > 248 ? fb->width - 248 : 32, 18, "tick: no timer yet", theme->muted_text, theme->top_bar);
}

static void hnm_router_render_help_overlay(void)
{
    const struct hnm_ui_theme *theme = hnm_ui_theme_default();
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    struct hnm_ui_panel panel;
    u32 width;
    u32 x;
    u32 y;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    width = fb->width > (HNM_ROUTER_HELP_MARGIN * 2) ?
        fb->width - (HNM_ROUTER_HELP_MARGIN * 2) :
        fb->width;
    x = fb->width > width ? (fb->width - width) / 2 : 0;
    y = fb->height > HNM_ROUTER_HELP_HEIGHT + HNM_ROUTER_HELP_MARGIN ?
        fb->height - HNM_ROUTER_HELP_HEIGHT - HNM_ROUTER_HELP_MARGIN :
        HNM_ROUTER_HELP_MARGIN;

    panel.rect.x = x;
    panel.rect.y = y;
    panel.rect.width = width;
    panel.rect.height = HNM_ROUTER_HELP_HEIGHT;
    panel.background = theme->panel;
    panel.border = theme->accent_alt;
    hnm_ui_panel_render(&panel);

    hnm_router_label(x + 16, y + 14, "Help", theme->text, theme->panel);
    hnm_router_label(x + 16, y + 38, "Launcher: 1 Terminal 2 System 3 Memory 5 AI 4 Shutdown", theme->muted_text, theme->panel);
    hnm_router_label(x + 16, y + 60, "ESC returns to Launcher. Terminal commands: back exit launcher.", theme->accent_alt, theme->panel);
}

static int hnm_router_handle_global_shortcut(const struct hnm_ui_event *event)
{
    if (hnm_router_current_screen == SCREEN_TERMINAL ||
        hnm_router_current_screen == SCREEN_LAUNCHER ||
        event->type != HNM_UI_EVENT_KEY_CHAR) {
        return 0;
    }

    if (event->character == '1') {
        ui_set_screen(SCREEN_TERMINAL);
        return 1;
    }

    if (event->character == '2') {
        ui_set_screen(SCREEN_SYSTEM);
        return 1;
    }

    if (event->character == '3') {
        ui_set_screen(SCREEN_MEMORY);
        return 1;
    }

    if (event->character == '4') {
        hnm_log_write_line("router: shutdown shortcut selected.");
        hnm_kernel_halt();
        return 1;
    }

    if (event->character == '5') {
        ui_set_screen(SCREEN_AI_PANEL_RESERVED);
        return 1;
    }

    return 0;
}

void ui_router_init(void)
{
    for (u32 i = 0; i < sizeof(hnm_router_screens) / sizeof(hnm_router_screens[0]); i++) {
        if (hnm_router_screens[i].screen_init != 0) {
            hnm_router_screens[i].screen_init();
        }
    }

    hnm_router_current_screen = SCREEN_LAUNCHER;
    hnm_router_ready = 1;

    if (hnm_router_current()->screen_on_enter != 0) {
        hnm_router_current()->screen_on_enter();
    }

    ui_render_current_screen();
    hnm_log_write_line("router: screen router ready.");
}

void ui_set_screen(enum hnm_screen_id screen_id)
{
    const struct hnm_screen *current;
    const struct hnm_screen *next;

    if (!hnm_router_ready) {
        return;
    }

    next = hnm_router_find_screen(screen_id);

    if (next == 0) {
        hnm_log_write_line("router: invalid screen requested.");
        return;
    }

    if (screen_id == hnm_router_current_screen) {
        ui_render_current_screen();
        return;
    }

    current = hnm_router_current();

    if (current != 0 && current->screen_on_exit != 0) {
        current->screen_on_exit();
    }

    hnm_router_current_screen = screen_id;

    if (next->screen_on_enter != 0) {
        next->screen_on_enter();
    }

    ui_render_current_screen();
}

void ui_back(void)
{
    if (hnm_router_current_screen == SCREEN_LAUNCHER) {
        ui_render_current_screen();
        return;
    }

    ui_set_screen(SCREEN_LAUNCHER);
}

void ui_render_current_screen(void)
{
    const struct hnm_screen *screen = hnm_router_current();

    if (!hnm_router_ready || screen == 0 || screen->screen_render == 0) {
        return;
    }

    if (screen->id != SCREEN_LAUNCHER && screen->id != SCREEN_TERMINAL) {
        hnm_router_render_top_bar();
    }

    screen->screen_render();
}

int ui_dispatch_event_to_current_screen(const struct hnm_ui_event *event)
{
    const struct hnm_screen *screen;

    if (!hnm_router_ready || event == 0) {
        return 0;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_ESCAPE &&
        hnm_router_current_screen != SCREEN_LAUNCHER) {
        ui_back();
        return 1;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_F1) {
        ui_render_current_screen();
        hnm_router_render_help_overlay();
        return 1;
    }

    if (hnm_router_handle_global_shortcut(event)) {
        return 1;
    }

    screen = hnm_router_current();

    if (screen == 0 || screen->screen_handle_event == 0) {
        return 0;
    }

    return screen->screen_handle_event(event);
}

enum hnm_screen_id ui_current_screen(void)
{
    return hnm_router_current_screen;
}

const char *ui_current_screen_name(void)
{
    const struct hnm_screen *screen = hnm_router_current();

    if (screen == 0) {
        return "Unknown";
    }

    return screen->name;
}

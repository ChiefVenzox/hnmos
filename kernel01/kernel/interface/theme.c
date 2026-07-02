#include "theme.h"

const struct hnm_ui_theme *hnm_ui_theme_default(void)
{
    static const struct hnm_ui_theme theme = {
        .background = 0x060A10,
        .top_bar = 0x0F1824,
        .panel = 0x101C29,
        .panel_alt = 0x162435,
        .border = 0x43DCD2,
        .accent = 0x847AFF,
        .accent_alt = 0x6AE88B,
        .text = 0xE8EEF5,
        .muted_text = 0x94A6B2,
        .button = 0x1B2F44,
        .button_hover = 0x244965,
        .button_pressed = 0x43DCD2,
        .button_text = 0xE8EEF5
    };

    return &theme;
}

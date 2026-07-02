#ifndef HNM_INTERFACE_THEME_H
#define HNM_INTERFACE_THEME_H

#include "../graphics/color.h"

struct hnm_ui_theme {
    hnm_color background;
    hnm_color top_bar;
    hnm_color panel;
    hnm_color panel_alt;
    hnm_color border;
    hnm_color accent;
    hnm_color accent_alt;
    hnm_color text;
    hnm_color muted_text;
    hnm_color button;
    hnm_color button_hover;
    hnm_color button_pressed;
    hnm_color button_text;
};

const struct hnm_ui_theme *hnm_ui_theme_default(void);

#endif

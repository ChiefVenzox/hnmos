#ifndef HNM_INTERFACE_WIDGETS_BUTTON_H
#define HNM_INTERFACE_WIDGETS_BUTTON_H

#include "../../graphics/color.h"
#include "rect.h"

struct hnm_ui_button {
    struct hnm_ui_rect rect;
    const char *text;
    hnm_color normal_background;
    hnm_color hover_background;
    hnm_color pressed_background;
    hnm_color border;
    hnm_color text_color;
    int hovered;
    int pressed;
};

void hnm_ui_button_render(const struct hnm_ui_button *button);
int hnm_ui_button_hit_test(const struct hnm_ui_button *button, u32 x, u32 y);

#endif

#ifndef HNM_INTERFACE_WIDGETS_WINDOW_H
#define HNM_INTERFACE_WIDGETS_WINDOW_H

#include "../../graphics/color.h"
#include "rect.h"

struct hnm_ui_window {
    struct hnm_ui_rect rect;
    const char *title;
    hnm_color title_background;
    hnm_color title_text;
    hnm_color body_background;
    hnm_color border;
};

void hnm_ui_window_render(const struct hnm_ui_window *window);
u32 hnm_ui_window_title_height(void);

#endif

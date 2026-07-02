#ifndef HNM_INTERFACE_WIDGETS_PANEL_H
#define HNM_INTERFACE_WIDGETS_PANEL_H

#include "../../graphics/color.h"
#include "rect.h"

struct hnm_ui_panel {
    struct hnm_ui_rect rect;
    hnm_color background;
    hnm_color border;
};

void hnm_ui_panel_render(const struct hnm_ui_panel *panel);

#endif

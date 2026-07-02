#ifndef HNM_INTERFACE_WIDGETS_LABEL_H
#define HNM_INTERFACE_WIDGETS_LABEL_H

#include "../../graphics/color.h"
#include "../../types.h"

struct hnm_ui_label {
    u32 x;
    u32 y;
    const char *text;
    hnm_color foreground;
    hnm_color background;
};

void hnm_ui_label_render(const struct hnm_ui_label *label);

#endif

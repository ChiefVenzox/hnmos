#ifndef HNM_INTERFACE_WIDGETS_RECT_H
#define HNM_INTERFACE_WIDGETS_RECT_H

#include "../../types.h"

struct hnm_ui_rect {
    u32 x;
    u32 y;
    u32 width;
    u32 height;
};

int hnm_ui_point_in_rect(u32 x, u32 y, const struct hnm_ui_rect *rect);

#endif

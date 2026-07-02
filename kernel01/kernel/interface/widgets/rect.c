#include "rect.h"

int hnm_ui_point_in_rect(u32 x, u32 y, const struct hnm_ui_rect *rect)
{
    u32 end_x;
    u32 end_y;

    if (rect->width == 0 || rect->height == 0) {
        return 0;
    }

    end_x = rect->x + rect->width;
    end_y = rect->y + rect->height;

    if (end_x < rect->x || end_y < rect->y) {
        return 0;
    }

    return x >= rect->x && x < end_x && y >= rect->y && y < end_y;
}

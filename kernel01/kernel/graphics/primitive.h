#ifndef HNM_GRAPHICS_PRIMITIVE_H
#define HNM_GRAPHICS_PRIMITIVE_H

#include "../types.h"
#include "color.h"

void hnm_draw_fill_rect(u32 x, u32 y, u32 width, u32 height, hnm_color color);
void hnm_draw_rect(u32 x, u32 y, u32 width, u32 height, hnm_color color);

#endif

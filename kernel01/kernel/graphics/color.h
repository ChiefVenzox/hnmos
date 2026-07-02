#ifndef HNM_GRAPHICS_COLOR_H
#define HNM_GRAPHICS_COLOR_H

#include "../types.h"

typedef u32 hnm_color;

hnm_color hnm_color_rgb(u8 red, u8 green, u8 blue);
u8 hnm_color_red(hnm_color color);
u8 hnm_color_green(hnm_color color);
u8 hnm_color_blue(hnm_color color);

#endif

#ifndef HNM_GRAPHICS_FONT_H
#define HNM_GRAPHICS_FONT_H

#include "../types.h"
#include "color.h"

enum {
    HNM_FONT_WIDTH = 8,
    HNM_FONT_HEIGHT = 8
};

void hnm_font_draw_char(u32 x, u32 y, char character, hnm_color fg, hnm_color bg);
void hnm_font_draw_string(u32 x, u32 y, const char *text, hnm_color fg, hnm_color bg);
u32 hnm_font_string_width(const char *text);

#endif

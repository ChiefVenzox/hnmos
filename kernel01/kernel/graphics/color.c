#include "color.h"

hnm_color hnm_color_rgb(u8 red, u8 green, u8 blue)
{
    return ((u32)red << 16) | ((u32)green << 8) | (u32)blue;
}

u8 hnm_color_red(hnm_color color)
{
    return (u8)((color >> 16) & 0xFF);
}

u8 hnm_color_green(hnm_color color)
{
    return (u8)((color >> 8) & 0xFF);
}

u8 hnm_color_blue(hnm_color color)
{
    return (u8)(color & 0xFF);
}

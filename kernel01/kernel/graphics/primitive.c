#include "primitive.h"
#include "framebuffer.h"

void hnm_draw_fill_rect(u32 x, u32 y, u32 width, u32 height, hnm_color color)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 end_x;
    u32 end_y;

    if (!hnm_framebuffer_is_available() || width == 0 || height == 0) {
        return;
    }

    if (x >= fb->width || y >= fb->height) {
        return;
    }

    end_x = x + width;
    end_y = y + height;

    if (end_x > fb->width || end_x < x) {
        end_x = fb->width;
    }

    if (end_y > fb->height || end_y < y) {
        end_y = fb->height;
    }

    for (u32 row = y; row < end_y; row++) {
        for (u32 col = x; col < end_x; col++) {
            hnm_framebuffer_put_pixel(col, row, color);
        }
    }
}

void hnm_draw_rect(u32 x, u32 y, u32 width, u32 height, hnm_color color)
{
    if (width == 0 || height == 0) {
        return;
    }

    hnm_draw_fill_rect(x, y, width, 1, color);
    hnm_draw_fill_rect(x, y + height - 1, width, 1, color);
    hnm_draw_fill_rect(x, y, 1, height, color);
    hnm_draw_fill_rect(x + width - 1, y, 1, height, color);
}

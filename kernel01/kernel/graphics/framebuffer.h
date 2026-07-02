#ifndef HNM_GRAPHICS_FRAMEBUFFER_H
#define HNM_GRAPHICS_FRAMEBUFFER_H

#include "../types.h"
#include "color.h"

enum hnm_framebuffer_source {
    HNM_FRAMEBUFFER_SOURCE_NONE = 0,
    HNM_FRAMEBUFFER_SOURCE_MULTIBOOT = 1,
    HNM_FRAMEBUFFER_SOURCE_BGA = 2
};

struct hnm_framebuffer {
    volatile u8 *buffer;
    u32 address;
    u32 width;
    u32 height;
    u32 pitch;
    u8 bpp;
    u8 type;
    u8 red_position;
    u8 red_mask_size;
    u8 green_position;
    u8 green_mask_size;
    u8 blue_position;
    u8 blue_mask_size;
    enum hnm_framebuffer_source source;
    int available;
};

int hnm_framebuffer_init(u32 multiboot_info_addr);
int hnm_framebuffer_is_available(void);
const struct hnm_framebuffer *hnm_framebuffer_get(void);
void hnm_framebuffer_log_info(void);
void hnm_framebuffer_put_pixel(u32 x, u32 y, hnm_color color);
void hnm_framebuffer_clear(hnm_color color);

#endif

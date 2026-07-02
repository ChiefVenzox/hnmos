#include "cursor.h"
#include "color.h"
#include "font.h"
#include "framebuffer.h"
#include "primitive.h"

enum {
    HNM_CURSOR_WIDTH = 12,
    HNM_CURSOR_HEIGHT = 16,
    HNM_CURSOR_MAX_BYTES_PER_PIXEL = 4,
    HNM_CURSOR_STATUS_WIDTH = 320,
    HNM_CURSOR_STATUS_HEIGHT = 20,
    HNM_CURSOR_STATUS_MARGIN = 8
};

static const u16 hnm_cursor_arrow[HNM_CURSOR_HEIGHT] = {
    0x800,
    0xC00,
    0xE00,
    0xF00,
    0xF80,
    0xFC0,
    0xFE0,
    0xFF0,
    0xFC0,
    0xDC0,
    0x8E0,
    0x060,
    0x030,
    0x030,
    0x018,
    0x000
};

static u8 hnm_cursor_saved_pixels[HNM_CURSOR_WIDTH * HNM_CURSOR_HEIGHT * HNM_CURSOR_MAX_BYTES_PER_PIXEL];
static u8 hnm_cursor_saved_valid[HNM_CURSOR_WIDTH * HNM_CURSOR_HEIGHT];
static u32 hnm_cursor_x;
static u32 hnm_cursor_y;
static int hnm_cursor_left_button;
static int hnm_cursor_right_button;
static int hnm_cursor_middle_button;
static int hnm_cursor_initialized;
static int hnm_cursor_visible;

static u32 hnm_cursor_bytes_per_pixel(const struct hnm_framebuffer *fb)
{
    return (u32)fb->bpp / 8;
}

static int hnm_cursor_body_pixel(int row, int col)
{
    if (row < 0 || row >= HNM_CURSOR_HEIGHT || col < 0 || col >= HNM_CURSOR_WIDTH) {
        return 0;
    }

    return (hnm_cursor_arrow[row] & (1 << (HNM_CURSOR_WIDTH - 1 - col))) != 0;
}

static int hnm_cursor_edge_pixel(int row, int col)
{
    if (!hnm_cursor_body_pixel(row, col)) {
        return 0;
    }

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (!hnm_cursor_body_pixel(row + dy, col + dx)) {
                return 1;
            }
        }
    }

    return 0;
}

static void hnm_cursor_append_char(char *text, u32 *index, u32 capacity, char value)
{
    if (*index + 1 >= capacity) {
        return;
    }

    text[*index] = value;
    *index = *index + 1;
}

static void hnm_cursor_append_text(char *text, u32 *index, u32 capacity, const char *value)
{
    for (u32 i = 0; value[i] != '\0'; i++) {
        hnm_cursor_append_char(text, index, capacity, value[i]);
    }
}

static void hnm_cursor_append_u32(char *text, u32 *index, u32 capacity, u32 value)
{
    char digits[10];
    u32 digit_count = 0;

    if (value == 0) {
        hnm_cursor_append_char(text, index, capacity, '0');
        return;
    }

    while (value != 0 && digit_count < 10) {
        digits[digit_count] = (char)('0' + (value % 10));
        value /= 10;
        digit_count++;
    }

    while (digit_count > 0) {
        digit_count--;
        hnm_cursor_append_char(text, index, capacity, digits[digit_count]);
    }
}

static void hnm_cursor_draw_status(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    hnm_color background = hnm_color_rgb(15, 24, 36);
    hnm_color border = hnm_color_rgb(67, 220, 210);
    hnm_color text_color = hnm_color_rgb(232, 238, 245);
    char text[64];
    u32 index = 0;
    u32 panel_x;
    u32 panel_y = 12;
    u32 text_x;
    u32 text_y = 18;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    panel_x = fb->width > HNM_CURSOR_STATUS_WIDTH + (HNM_CURSOR_STATUS_MARGIN * 2) ?
        fb->width - HNM_CURSOR_STATUS_WIDTH - HNM_CURSOR_STATUS_MARGIN :
        HNM_CURSOR_STATUS_MARGIN;
    text_x = panel_x + 8;

    hnm_cursor_append_text(text, &index, sizeof(text), "mouse x:");
    hnm_cursor_append_u32(text, &index, sizeof(text), hnm_cursor_x);
    hnm_cursor_append_text(text, &index, sizeof(text), " y:");
    hnm_cursor_append_u32(text, &index, sizeof(text), hnm_cursor_y);
    hnm_cursor_append_text(text, &index, sizeof(text), " l:");
    hnm_cursor_append_char(text, &index, sizeof(text), hnm_cursor_left_button ? '1' : '0');
    hnm_cursor_append_text(text, &index, sizeof(text), " r:");
    hnm_cursor_append_char(text, &index, sizeof(text), hnm_cursor_right_button ? '1' : '0');
    hnm_cursor_append_text(text, &index, sizeof(text), " m:");
    hnm_cursor_append_char(text, &index, sizeof(text), hnm_cursor_middle_button ? '1' : '0');
    text[index] = '\0';

    hnm_draw_fill_rect(panel_x, panel_y, HNM_CURSOR_STATUS_WIDTH, HNM_CURSOR_STATUS_HEIGHT, background);
    hnm_draw_rect(panel_x, panel_y, HNM_CURSOR_STATUS_WIDTH, HNM_CURSOR_STATUS_HEIGHT, border);
    hnm_font_draw_string(text_x, text_y, text, text_color, background);
}

static void hnm_cursor_save_background(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 bytes_per_pixel;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    bytes_per_pixel = hnm_cursor_bytes_per_pixel(fb);

    for (u32 row = 0; row < HNM_CURSOR_HEIGHT; row++) {
        for (u32 col = 0; col < HNM_CURSOR_WIDTH; col++) {
            u32 saved_index = row * HNM_CURSOR_WIDTH + col;
            u32 pixel_x = hnm_cursor_x + col;
            u32 pixel_y = hnm_cursor_y + row;
            u32 byte_index = saved_index * HNM_CURSOR_MAX_BYTES_PER_PIXEL;
            volatile u8 *pixel;

            hnm_cursor_saved_valid[saved_index] = 0;

            if (pixel_x >= fb->width || pixel_y >= fb->height) {
                continue;
            }

            pixel = fb->buffer + (pixel_y * fb->pitch) + (pixel_x * bytes_per_pixel);

            for (u32 byte = 0; byte < bytes_per_pixel; byte++) {
                hnm_cursor_saved_pixels[byte_index + byte] = pixel[byte];
            }

            hnm_cursor_saved_valid[saved_index] = 1;
        }
    }
}

static void hnm_cursor_draw_shape(void)
{
    hnm_color outline = hnm_color_rgb(5, 8, 13);
    hnm_color fill = hnm_color_rgb(248, 252, 255);

    for (int row = 0; row < HNM_CURSOR_HEIGHT; row++) {
        for (int col = 0; col < HNM_CURSOR_WIDTH; col++) {
            if (!hnm_cursor_body_pixel(row, col)) {
                continue;
            }

            hnm_framebuffer_put_pixel(
                hnm_cursor_x + (u32)col,
                hnm_cursor_y + (u32)row,
                hnm_cursor_edge_pixel(row, col) ? outline : fill);
        }
    }
}

void hnm_cursor_init(u32 x, u32 y)
{
    if (!hnm_framebuffer_is_available()) {
        hnm_cursor_initialized = 0;
        hnm_cursor_visible = 0;
        return;
    }

    hnm_cursor_x = x;
    hnm_cursor_y = y;
    hnm_cursor_left_button = 0;
    hnm_cursor_right_button = 0;
    hnm_cursor_middle_button = 0;
    hnm_cursor_initialized = 1;
    hnm_cursor_visible = 0;
    hnm_cursor_show_current();
}

void hnm_cursor_hide(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 bytes_per_pixel;

    if (!hnm_cursor_visible || !hnm_framebuffer_is_available()) {
        hnm_cursor_visible = 0;
        return;
    }

    bytes_per_pixel = hnm_cursor_bytes_per_pixel(fb);

    for (u32 row = 0; row < HNM_CURSOR_HEIGHT; row++) {
        for (u32 col = 0; col < HNM_CURSOR_WIDTH; col++) {
            u32 saved_index = row * HNM_CURSOR_WIDTH + col;
            u32 pixel_x = hnm_cursor_x + col;
            u32 pixel_y = hnm_cursor_y + row;
            u32 byte_index = saved_index * HNM_CURSOR_MAX_BYTES_PER_PIXEL;
            volatile u8 *pixel;

            if (!hnm_cursor_saved_valid[saved_index] || pixel_x >= fb->width || pixel_y >= fb->height) {
                continue;
            }

            pixel = fb->buffer + (pixel_y * fb->pitch) + (pixel_x * bytes_per_pixel);

            for (u32 byte = 0; byte < bytes_per_pixel; byte++) {
                pixel[byte] = hnm_cursor_saved_pixels[byte_index + byte];
            }
        }
    }

    hnm_cursor_visible = 0;
}

void hnm_cursor_show_current(void)
{
    if (!hnm_cursor_initialized || hnm_cursor_visible || !hnm_framebuffer_is_available()) {
        return;
    }

    hnm_cursor_draw_status();
    hnm_cursor_save_background();
    hnm_cursor_draw_shape();
    hnm_cursor_visible = 1;
}

void hnm_cursor_move(u32 x, u32 y, int left_button, int right_button, int middle_button)
{
    if (!hnm_cursor_initialized || !hnm_framebuffer_is_available()) {
        return;
    }

    hnm_cursor_hide();
    hnm_cursor_x = x;
    hnm_cursor_y = y;
    hnm_cursor_left_button = left_button;
    hnm_cursor_right_button = right_button;
    hnm_cursor_middle_button = middle_button;
    hnm_cursor_show_current();
}

void hnm_cursor_invalidate(void)
{
    hnm_cursor_visible = 0;
}

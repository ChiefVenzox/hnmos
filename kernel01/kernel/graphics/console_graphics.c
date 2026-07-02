#include "console_graphics.h"
#include "cursor.h"
#include "font.h"
#include "framebuffer.h"
#include "primitive.h"

struct hnm_graphics_console {
    u32 cursor_x;
    u32 cursor_y;
    u32 columns;
    u32 rows;
    hnm_color foreground;
    hnm_color background;
    int ready;
};

static struct hnm_graphics_console hnm_console;

static u32 hnm_console_pixel_x(void)
{
    return hnm_console.cursor_x * HNM_FONT_WIDTH;
}

static u32 hnm_console_pixel_y(void)
{
    return hnm_console.cursor_y * HNM_FONT_HEIGHT;
}

void hnm_console_graphics_init(hnm_color foreground, hnm_color background)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();

    hnm_console.cursor_x = 0;
    hnm_console.cursor_y = 0;
    hnm_console.columns = 0;
    hnm_console.rows = 0;
    hnm_console.foreground = foreground;
    hnm_console.background = background;
    hnm_console.ready = 0;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_console.columns = fb->width / HNM_FONT_WIDTH;
    hnm_console.rows = fb->height / HNM_FONT_HEIGHT;
    hnm_console.ready = hnm_console.columns > 0 && hnm_console.rows > 0;
}

int hnm_console_graphics_is_ready(void)
{
    return hnm_console.ready;
}

void hnm_console_set_colors(hnm_color foreground, hnm_color background)
{
    hnm_console.foreground = foreground;
    hnm_console.background = background;
}

void hnm_console_set_cursor(u32 column, u32 row)
{
    if (!hnm_console.ready) {
        return;
    }

    if (column >= hnm_console.columns) {
        column = hnm_console.columns - 1;
    }

    if (row >= hnm_console.rows) {
        row = hnm_console.rows - 1;
    }

    hnm_console.cursor_x = column;
    hnm_console.cursor_y = row;
}

void hnm_console_clear(void)
{
    if (!hnm_console.ready) {
        return;
    }

    hnm_framebuffer_clear(hnm_console.background);
    hnm_cursor_invalidate();
    hnm_console.cursor_x = 0;
    hnm_console.cursor_y = 0;
}

static void hnm_console_scroll(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 bytes_per_pixel;
    u32 visible_row_bytes;
    u32 scroll_pixels = HNM_FONT_HEIGHT;

    if (!hnm_console.ready || fb->height <= scroll_pixels) {
        return;
    }

    bytes_per_pixel = (u32)fb->bpp / 8;
    visible_row_bytes = fb->width * bytes_per_pixel;

    for (u32 y = 0; y < fb->height - scroll_pixels; y++) {
        volatile u8 *to = fb->buffer + (y * fb->pitch);
        volatile u8 *from = fb->buffer + ((y + scroll_pixels) * fb->pitch);

        for (u32 byte = 0; byte < visible_row_bytes; byte++) {
            to[byte] = from[byte];
        }
    }

    hnm_draw_fill_rect(0, fb->height - scroll_pixels, fb->width, scroll_pixels, hnm_console.background);
    hnm_cursor_invalidate();
}

void hnm_console_newline(void)
{
    if (!hnm_console.ready) {
        return;
    }

    hnm_console.cursor_x = 0;

    if (hnm_console.cursor_y + 1 >= hnm_console.rows) {
        hnm_console_scroll();
        hnm_console.cursor_y = hnm_console.rows - 1;
        return;
    }

    hnm_console.cursor_y++;
}

void hnm_console_backspace(void)
{
    if (!hnm_console.ready) {
        return;
    }

    if (hnm_console.cursor_x == 0) {
        if (hnm_console.cursor_y == 0) {
            return;
        }

        hnm_console.cursor_y--;
        hnm_console.cursor_x = hnm_console.columns - 1;
    } else {
        hnm_console.cursor_x--;
    }

    hnm_draw_fill_rect(
        hnm_console_pixel_x(),
        hnm_console_pixel_y(),
        HNM_FONT_WIDTH,
        HNM_FONT_HEIGHT,
        hnm_console.background);
}

void hnm_console_put_char(char character)
{
    if (!hnm_console.ready) {
        return;
    }

    if (character == '\n') {
        hnm_console_newline();
        return;
    }

    if (character == '\r') {
        hnm_console.cursor_x = 0;
        return;
    }

    if (character == '\b') {
        hnm_console_backspace();
        return;
    }

    if (character == '\t') {
        for (u32 i = 0; i < 4; i++) {
            hnm_console_put_char(' ');
        }

        return;
    }

    if (hnm_console.cursor_x >= hnm_console.columns) {
        hnm_console_newline();
    }

    hnm_font_draw_char(
        hnm_console_pixel_x(),
        hnm_console_pixel_y(),
        character,
        hnm_console.foreground,
        hnm_console.background);

    hnm_console.cursor_x++;

    if (hnm_console.cursor_x >= hnm_console.columns) {
        hnm_console_newline();
    }
}

void hnm_console_write(const char *text)
{
    for (u32 i = 0; text[i] != '\0'; i++) {
        hnm_console_put_char(text[i]);
    }
}

void hnm_console_write_line(const char *text)
{
    hnm_console_write(text);
    hnm_console_newline();
}

void hnm_console_graphics_draw_boot_screen(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    hnm_color background = hnm_color_rgb(6, 10, 16);
    hnm_color panel = hnm_color_rgb(15, 24, 36);
    hnm_color border = hnm_color_rgb(67, 220, 210);
    hnm_color text = hnm_color_rgb(232, 238, 245);
    hnm_color muted = hnm_color_rgb(148, 166, 178);
    hnm_color ok = hnm_color_rgb(106, 232, 139);
    hnm_color warn = hnm_color_rgb(246, 190, 83);
    hnm_color accent = hnm_color_rgb(132, 122, 255);

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    hnm_console_graphics_init(text, background);

    if (!hnm_console_graphics_is_ready()) {
        return;
    }

    hnm_console_clear();

    hnm_draw_fill_rect(0, 0, fb->width, 80, panel);
    hnm_draw_fill_rect(0, 80, fb->width, 4, border);

    hnm_console_set_colors(text, panel);
    hnm_console_set_cursor(4, 2);
    hnm_console_write("HNMos v0.3");

    hnm_console_set_colors(muted, panel);
    hnm_console_set_cursor(4, 5);
    hnm_console_write("Framebuffer text mode");

    hnm_console_set_colors(text, background);
    hnm_console_set_cursor(4, 12);
    hnm_console_write_line("Graphical Console Online");

    hnm_console_set_colors(ok, background);
    hnm_console_set_cursor(4, 15);
    hnm_console_write("Kernel: OK");
    hnm_console_set_cursor(4, 17);
    hnm_console_write("Framebuffer: Ready");
    hnm_console_set_cursor(4, 19);
    hnm_console_write("Font: Ready");

    hnm_draw_fill_rect(32, 184, 128, 12, ok);
    hnm_draw_fill_rect(176, 184, 128, 12, warn);
    hnm_draw_fill_rect(320, 184, 128, 12, border);
    hnm_draw_fill_rect(464, 184, 128, 12, accent);
}

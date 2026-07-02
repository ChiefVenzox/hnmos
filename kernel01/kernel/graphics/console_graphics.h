#ifndef HNM_GRAPHICS_CONSOLE_GRAPHICS_H
#define HNM_GRAPHICS_CONSOLE_GRAPHICS_H

#include "../types.h"
#include "color.h"

void hnm_console_graphics_init(hnm_color foreground, hnm_color background);
int hnm_console_graphics_is_ready(void);
void hnm_console_set_colors(hnm_color foreground, hnm_color background);
void hnm_console_set_cursor(u32 column, u32 row);
void hnm_console_clear(void);
void hnm_console_put_char(char character);
void hnm_console_write(const char *text);
void hnm_console_write_line(const char *text);
void hnm_console_newline(void);
void hnm_console_backspace(void);
void hnm_console_graphics_draw_boot_screen(void);

#endif

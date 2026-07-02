#ifndef HNM_GRAPHICS_CURSOR_H
#define HNM_GRAPHICS_CURSOR_H

#include "../types.h"

void hnm_cursor_init(u32 x, u32 y);
void hnm_cursor_hide(void);
void hnm_cursor_show_current(void);
void hnm_cursor_move(u32 x, u32 y, int left_button, int right_button, int middle_button);
void hnm_cursor_invalidate(void);

#endif

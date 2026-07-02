#ifndef HNM_INTERFACE_SCREENS_MEMORY_SCREEN_H
#define HNM_INTERFACE_SCREENS_MEMORY_SCREEN_H

#include "../events.h"

void hnm_memory_screen_init(void);
void hnm_memory_screen_render(void);
int hnm_memory_screen_handle_event(const struct hnm_ui_event *event);
void hnm_memory_screen_on_enter(void);
void hnm_memory_screen_on_exit(void);

#endif

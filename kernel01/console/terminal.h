#ifndef HNM_TERMINAL_H
#define HNM_TERMINAL_H

#include "../kernel/interface/events.h"

void hnm_terminal_init(void);
void hnm_terminal_poll(void);
void hnm_terminal_handle_event(const struct hnm_ui_event *event);
void hnm_terminal_show_screen(void);

#endif

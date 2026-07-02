#ifndef HNM_INTERFACE_LAUNCHER_H
#define HNM_INTERFACE_LAUNCHER_H

#include "events.h"

void hnm_launcher_init(void);
void hnm_launcher_render(void);
int hnm_launcher_handle_event(const struct hnm_ui_event *event);
void hnm_launcher_on_enter(void);
void hnm_launcher_on_exit(void);

#endif

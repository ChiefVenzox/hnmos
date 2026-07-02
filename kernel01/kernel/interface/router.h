#ifndef HNM_INTERFACE_ROUTER_H
#define HNM_INTERFACE_ROUTER_H

#include "events.h"
#include "screen.h"

void ui_router_init(void);
void ui_set_screen(enum hnm_screen_id screen_id);
void ui_back(void);
void ui_render_current_screen(void);
int ui_dispatch_event_to_current_screen(const struct hnm_ui_event *event);
enum hnm_screen_id ui_current_screen(void);
const char *ui_current_screen_name(void);

#endif

#ifndef HNM_INTERFACE_SCREENS_AI_STUDIO_SCREEN_H
#define HNM_INTERFACE_SCREENS_AI_STUDIO_SCREEN_H

#include "../events.h"

void hnm_ai_studio_screen_init(void);
void hnm_ai_studio_screen_render(void);
int hnm_ai_studio_screen_handle_event(const struct hnm_ui_event *event);
void hnm_ai_studio_screen_on_enter(void);
void hnm_ai_studio_screen_on_exit(void);

#endif

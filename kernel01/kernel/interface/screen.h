#ifndef HNM_INTERFACE_SCREEN_H
#define HNM_INTERFACE_SCREEN_H

#include "events.h"

enum hnm_screen_id {
    SCREEN_LAUNCHER = 0,
    SCREEN_TERMINAL,
    SCREEN_SYSTEM,
    SCREEN_MEMORY,
    SCREEN_AI_PANEL_RESERVED,
    SCREEN_COUNT
};

struct hnm_screen {
    enum hnm_screen_id id;
    const char *name;
    void (*screen_init)(void);
    void (*screen_render)(void);
    int (*screen_handle_event)(const struct hnm_ui_event *event);
    void (*screen_on_enter)(void);
    void (*screen_on_exit)(void);
};

#endif

#ifndef HNM_INTERFACE_EVENTS_H
#define HNM_INTERFACE_EVENTS_H

#include "../types.h"

enum hnm_ui_event_type {
    HNM_UI_EVENT_NONE = 0,
    HNM_UI_EVENT_KEY_DOWN,
    HNM_UI_EVENT_KEY_CHAR,
    HNM_UI_EVENT_MOUSE_MOVE,
    HNM_UI_EVENT_MOUSE_BUTTON_DOWN,
    HNM_UI_EVENT_MOUSE_BUTTON_UP
};

enum hnm_ui_key_code {
    HNM_UI_KEY_NONE = 0,
    HNM_UI_KEY_ENTER = 13,
    HNM_UI_KEY_BACKSPACE = 8,
    HNM_UI_KEY_ESCAPE = 27,
    HNM_UI_KEY_F1 = 0x3B
};

enum hnm_ui_mouse_button {
    HNM_UI_MOUSE_BUTTON_NONE = 0,
    HNM_UI_MOUSE_BUTTON_LEFT = 1,
    HNM_UI_MOUSE_BUTTON_RIGHT = 2,
    HNM_UI_MOUSE_BUTTON_MIDDLE = 3
};

enum hnm_ui_mouse_button_mask {
    HNM_UI_MOUSE_LEFT_MASK = 1 << 0,
    HNM_UI_MOUSE_RIGHT_MASK = 1 << 1,
    HNM_UI_MOUSE_MIDDLE_MASK = 1 << 2
};

struct hnm_ui_event {
    enum hnm_ui_event_type type;
    u32 sequence;
    u32 x;
    u32 y;
    int dx;
    int dy;
    u32 key_code;
    u8 scancode;
    u8 button;
    u8 buttons;
    char character;
};

void hnm_ui_events_init(void);
void hnm_ui_event_init(struct hnm_ui_event *event, enum hnm_ui_event_type type);
int hnm_ui_event_push(const struct hnm_ui_event *event);
int hnm_ui_event_poll(struct hnm_ui_event *event);
int hnm_ui_event_recent(struct hnm_ui_event *event);
u32 hnm_ui_event_pending_count(void);
u32 hnm_ui_event_dropped_count(void);

#endif

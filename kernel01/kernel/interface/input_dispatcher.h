#ifndef HNM_INTERFACE_INPUT_DISPATCHER_H
#define HNM_INTERFACE_INPUT_DISPATCHER_H

#include "events.h"

typedef void (*hnm_input_event_handler)(const struct hnm_ui_event *event);

void hnm_input_dispatcher_init(void);
void hnm_input_dispatcher_dispatch(hnm_input_event_handler handler);

#endif

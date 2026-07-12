#ifndef HNM_AI_SERIAL_TRANSPORT_H
#define HNM_AI_SERIAL_TRANSPORT_H

#include "ai_task.h"

void hnm_ai_serial_transport_init(void);
int hnm_ai_serial_transport_send_api_key(const char *api_key);
int hnm_ai_serial_transport_send(const struct hnm_ai_task *task);
int hnm_ai_serial_transport_poll(struct hnm_ai_task *task);
void hnm_ai_serial_irq_handler(void);
u32 hnm_ai_serial_dropped_bytes(void);

#endif

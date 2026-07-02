#ifndef HNM_DRIVERS_MOUSE_H
#define HNM_DRIVERS_MOUSE_H

int hnm_mouse_init(void);
int hnm_mouse_is_ready(void);
void hnm_mouse_irq_handler(void);
void hnm_mouse_poll(void);

#endif

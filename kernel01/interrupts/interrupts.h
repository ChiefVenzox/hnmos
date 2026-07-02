#ifndef HNM_INTERRUPTS_H
#define HNM_INTERRUPTS_H

void hnm_interrupts_init(void);
void hnm_interrupts_idle(void) __attribute__((noreturn));
void hnm_interrupts_enable(void);
void hnm_interrupts_disable(void);
void hnm_pic_send_eoi(unsigned char irq);

#endif

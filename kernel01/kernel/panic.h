#ifndef HNM_PANIC_H
#define HNM_PANIC_H

void hnm_kernel_halt(void) __attribute__((noreturn));
void hnm_panic(const char *message) __attribute__((noreturn));

#endif

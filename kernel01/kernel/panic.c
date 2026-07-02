#include "log.h"
#include "panic.h"

void hnm_kernel_halt(void)
{
    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}

void hnm_panic(const char *message)
{
    hnm_log_write_line("");
    hnm_log_write_line("HNMos kernel panic.");
    hnm_log_write("reason: ");
    hnm_log_write_line(message);

    hnm_kernel_halt();
}

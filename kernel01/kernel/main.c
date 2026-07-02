#include "log.h"
#include "panic.h"
#include "types.h"
#include "../console/terminal.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../interrupts/interrupts.h"
#include "ai/ai_bridge.h"
#include "graphics/console_graphics.h"
#include "graphics/cursor.h"
#include "graphics/framebuffer.h"
#include "fs/vfs.h"
#include "interface/events.h"
#include "interface/input_dispatcher.h"
#include "interface/router.h"
#include "memory/heap.h"
#include "memory/pmm.h"
#include "task/scheduler.h"

enum {
    HNM_MULTIBOOT_BOOTLOADER_MAGIC = 0x2BADB002
};

static void hnm_kernel_handle_input_event(const struct hnm_ui_event *event)
{
    ui_dispatch_event_to_current_screen(event);
}

static void hnm_kernel_memory_self_check(void)
{
    void *first = hnm_kmalloc(32);
    void *second = hnm_kmalloc(64);

    if (first != 0 && second != 0) {
        hnm_log_write_line("heap: smoke allocation ok.");
    } else {
        hnm_log_write_line("heap: smoke allocation failed.");
    }
}

void hnm_kernel_main(u32 multiboot_magic, u32 multiboot_info_addr)
{
    int graphics_ready;

    hnm_log_init();

    hnm_log_write_line("HNMos Kernel 01 initialized.");
    hnm_log_write_line("AI-native operating layer will be loaded later.");
    hnm_log_write_line("HNMos v0.0.1 demo boot.");
    hnm_log_write_line("");
    hnm_log_write_line("boot mode: bare-metal kernel");
    hnm_log_write("multiboot magic: ");
    hnm_log_write_hex32(multiboot_magic);
    hnm_log_write_line("");
    hnm_log_write("multiboot info:  ");
    hnm_log_write_hex32(multiboot_info_addr);
    hnm_log_write_line("");

    if (multiboot_magic != HNM_MULTIBOOT_BOOTLOADER_MAGIC) {
        hnm_panic("invalid multiboot magic");
    }

    graphics_ready = hnm_framebuffer_init(multiboot_info_addr);

    if (graphics_ready) {
        hnm_framebuffer_log_info();
        hnm_console_graphics_draw_boot_screen();
        hnm_log_write_line("graphics: graphical console boot screen rendered.");
    } else {
        hnm_log_write_line("graphics: framebuffer unavailable; using idle fallback.");
    }

    hnm_pmm_init(multiboot_info_addr);
    hnm_heap_init();
    hnm_kernel_memory_self_check();
    hnm_heap_log_stats();
    hnm_fs_init();
    hnm_scheduler_init();
    hnm_ai_bridge_init();

    hnm_log_write_line("status: Kernel 01 is idle.");
    hnm_log_write_line("");
    hnm_log_write_line("console: minimal output console ready.");
    hnm_log_write_line("hnshell: offline demo stub ready.");
    hnm_log_write_line("ai-runtime: OS interface stub ready; no model loaded.");
    hnm_log_write_line("task-demo: hn status -> Kernel 01 idle.");
    hnm_log_write_line("task-demo: hn ai status -> AI runtime stub offline.");
    hnm_log_write_line("task-demo: hn task list -> demo task queue empty.");

    if (!graphics_ready) {
        hnm_kernel_halt();
    }

    hnm_ui_events_init();
    hnm_input_dispatcher_init();
    hnm_keyboard_init();
    hnm_mouse_init();
    hnm_interrupts_init();
    hnm_cursor_hide();
    hnm_terminal_init();
    ui_router_init();
    hnm_cursor_show_current();
    hnm_log_write_line("terminal: hnmos terminal available.");

    for (;;) {
        hnm_interrupts_disable();
        hnm_cursor_hide();
        hnm_input_dispatcher_dispatch(hnm_kernel_handle_input_event);
        hnm_mouse_poll();
        hnm_cursor_show_current();
        hnm_scheduler_yield();
        __asm__ volatile ("sti; hlt");
    }
}

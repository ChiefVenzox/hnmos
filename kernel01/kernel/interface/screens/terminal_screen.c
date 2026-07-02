#include "terminal_screen.h"
#include "../../../console/terminal.h"
#include "../../log.h"

void hnm_terminal_screen_init(void)
{
}

void hnm_terminal_screen_render(void)
{
    hnm_terminal_show_screen();
}

int hnm_terminal_screen_handle_event(const struct hnm_ui_event *event)
{
    if (event->type == HNM_UI_EVENT_KEY_CHAR ||
        event->type == HNM_UI_EVENT_KEY_DOWN) {
        hnm_terminal_handle_event(event);
        return 1;
    }

    return 0;
}

void hnm_terminal_screen_on_enter(void)
{
    hnm_log_write_line("router: terminal screen entered.");
}

void hnm_terminal_screen_on_exit(void)
{
    hnm_log_write_line("router: terminal screen exited.");
}

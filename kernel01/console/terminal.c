#include "terminal.h"
#include "../kernel/log.h"
#include "../kernel/panic.h"
#include "../kernel/types.h"
#include "../kernel/ai/ai_bridge.h"
#include "../kernel/fs/vfs.h"
#include "../kernel/graphics/color.h"
#include "../kernel/graphics/console_graphics.h"
#include "../kernel/interface/input_dispatcher.h"
#include "../kernel/interface/router.h"
#include "../kernel/task/scheduler.h"

enum {
    HNM_TERMINAL_INPUT_MAX = 80
};

struct hnm_terminal_idt_pointer {
    u16 limit;
    u32 base;
} __attribute__((packed));

static char hnm_terminal_input[HNM_TERMINAL_INPUT_MAX];
static u32 hnm_terminal_input_length;
static hnm_color hnm_terminal_fg;
static hnm_color hnm_terminal_bg;
static hnm_color hnm_terminal_accent;

static int hnm_string_equal(const char *left, const char *right)
{
    u32 i = 0;

    while (left[i] != '\0' && right[i] != '\0') {
        if (left[i] != right[i]) {
            return 0;
        }

        i++;
    }

    return left[i] == right[i];
}

static int hnm_terminal_command_matches(const char *command, const char *name)
{
    u32 i = 0;

    while (name[i] != '\0') {
        if (command[i] != name[i]) {
            return 0;
        }

        i++;
    }

    return command[i] == '\0' || command[i] == ' ';
}

static const char *hnm_terminal_command_argument(const char *command, const char *name)
{
    u32 index = 0;

    while (name[index] != '\0') {
        index++;
    }

    while (command[index] == ' ') {
        index++;
    }

    return &command[index];
}

static void hnm_terminal_copy_path(const char *argument, char *path, u32 capacity)
{
    u32 read_index = 0;
    u32 write_index = 0;

    if (capacity == 0) {
        return;
    }

    if (argument == 0 || argument[0] == '\0') {
        if (capacity > 1) {
            path[0] = '/';
            path[1] = '\0';
        } else {
            path[0] = '\0';
        }

        return;
    }

    if (argument[0] != '/' && write_index + 1 < capacity) {
        path[write_index] = '/';
        write_index++;
    }

    while (argument[read_index] != '\0' &&
        argument[read_index] != ' ' &&
        write_index + 1 < capacity) {
        path[write_index] = argument[read_index];
        write_index++;
        read_index++;
    }

    path[write_index] = '\0';
}

static void hnm_terminal_print_node_line(const struct hnm_fs_node *node)
{
    hnm_console_write(hnm_fs_node_type_name(node->type));
    hnm_console_write(" ");
    hnm_console_write_line(node->name);
}

static void hnm_terminal_write_u32(u32 value)
{
    char digits[10];
    u32 digit_count = 0;

    if (value == 0) {
        hnm_console_put_char('0');
        return;
    }

    while (value != 0 && digit_count < 10) {
        digits[digit_count] = (char)('0' + (value % 10));
        value /= 10;
        digit_count++;
    }

    while (digit_count > 0) {
        digit_count--;
        hnm_console_put_char(digits[digit_count]);
    }
}

static void hnm_terminal_prompt(void)
{
    hnm_console_set_colors(hnm_terminal_accent, hnm_terminal_bg);
    hnm_console_write("hnmos> ");
    hnm_console_set_colors(hnm_terminal_fg, hnm_terminal_bg);
}

static void hnm_terminal_return_to_launcher(const char *command)
{
    hnm_log_write("terminal command: ");
    hnm_log_write_line(command);
    ui_set_screen(SCREEN_LAUNCHER);
}

static void hnm_terminal_reboot(void)
{
    struct hnm_terminal_idt_pointer empty_idt = {0, 0};

    hnm_log_write_line("terminal command: reboot");
    hnm_console_write_line("rebooting");

    for (volatile u32 delay = 0; delay < 10000; delay++) {
    }

    hnm_log_write_line("terminal reboot: triple fault reset");
    __asm__ volatile ("cli; lidt %0; int $3" : : "m"(empty_idt));
    hnm_kernel_halt();
}

static void hnm_terminal_execute_ls(const char *argument)
{
    struct hnm_fs_list list;
    char path[64];

    hnm_log_write_line("terminal command: ls");
    hnm_terminal_copy_path(argument, path, sizeof(path));

    if (!hnm_fs_list(path, &list)) {
        hnm_console_write("ls: not found: ");
        hnm_console_write_line(path);
        hnm_terminal_prompt();
        return;
    }

    hnm_console_write(path);
    hnm_console_write_line(":");

    if (list.count == 0) {
        hnm_console_write_line("empty");
    }

    for (u32 i = 0; i < list.count; i++) {
        hnm_terminal_print_node_line(list.nodes[i]);
    }

    if (list.truncated) {
        hnm_console_write_line("list truncated");
    }

    hnm_terminal_prompt();
}

static void hnm_terminal_execute_cat(const char *argument)
{
    const struct hnm_fs_node *node;
    char path[64];

    hnm_log_write_line("terminal command: cat");

    if (argument == 0 || argument[0] == '\0') {
        hnm_console_write_line("cat: missing path");
        hnm_terminal_prompt();
        return;
    }

    hnm_terminal_copy_path(argument, path, sizeof(path));

    if (!hnm_fs_read(path, &node)) {
        hnm_console_write("cat: not found: ");
        hnm_console_write_line(path);
        hnm_terminal_prompt();
        return;
    }

    if (node->type != HNM_FS_NODE_FILE) {
        hnm_console_write("cat: not a file: ");
        hnm_console_write_line(path);
        hnm_terminal_prompt();
        return;
    }

    if (node->data != 0 && node->size > 0) {
        hnm_console_write(node->data);

        if (node->data[node->size - 1] != '\n') {
            hnm_console_newline();
        }
    } else {
        hnm_console_newline();
    }

    hnm_terminal_prompt();
}

static void hnm_terminal_execute_tasks(void)
{
    struct hnm_scheduler_stats stats;

    hnm_log_write_line("terminal command: tasks");
    hnm_scheduler_get_stats(&stats);

    hnm_console_write_line("tasks:");
    hnm_console_write("scheduler: ");
    hnm_console_write(stats.ready ? "ready" : "not ready");
    hnm_console_write(" yields ");
    hnm_terminal_write_u32(stats.yield_count);
    hnm_console_newline();

    for (u32 i = 0; i < hnm_task_count(); i++) {
        const struct hnm_task *task = hnm_task_get(i);

        if (task == 0) {
            continue;
        }

        hnm_terminal_write_u32(task->id);
        hnm_console_write(" ");
        hnm_console_write(task->name);
        hnm_console_write(" ");
        hnm_console_write(hnm_task_state_name(task->state));
        hnm_console_write(" yields ");
        hnm_terminal_write_u32(task->yield_count);
        hnm_console_newline();
    }

    hnm_terminal_prompt();
}

static void hnm_terminal_execute_ai(const char *argument)
{
    const char *response;

    hnm_log_write("terminal command: ai ");
    hnm_log_write_line(argument != 0 && argument[0] != '\0' ? argument : "unknown");

    if (hnm_string_equal(argument, "help")) {
        response = hnm_ai_bridge_request_help();
        hnm_console_write_line(response);
        hnm_log_write_line(response);
        hnm_terminal_prompt();
        return;
    }

    if (hnm_string_equal(argument, "sysinfo")) {
        response = hnm_ai_bridge_request_sysinfo();
        hnm_console_write_line(response);
        hnm_log_write_line(response);
        hnm_terminal_prompt();
        return;
    }

    if (hnm_string_equal(argument, "status")) {
        response = hnm_ai_bridge_request_status();
        hnm_console_write_line(response);
        hnm_log_write_line(response);
        hnm_console_write("provider: ");
        hnm_console_write_line(hnm_ai_bridge_provider_name());
        hnm_console_write("provider type: ");
        hnm_console_write_line(hnm_ai_bridge_provider_type_name());
        hnm_terminal_prompt();
        return;
    }

    hnm_console_write_line("ai commands: ai help, ai sysinfo, ai status");
    hnm_terminal_prompt();
}

static void hnm_terminal_execute_command(const char *command)
{
    if (command[0] == '\0') {
        hnm_terminal_prompt();
        return;
    }

    if (hnm_string_equal(command, "help")) {
        hnm_log_write_line("terminal command: help");
        hnm_console_write_line("commands: help version clear halt reboot ls cat tasks ai back exit launcher");
        hnm_terminal_prompt();
        return;
    }

    if (hnm_terminal_command_matches(command, "ai")) {
        hnm_terminal_execute_ai(hnm_terminal_command_argument(command, "ai"));
        return;
    }

    if (hnm_string_equal(command, "tasks")) {
        hnm_terminal_execute_tasks();
        return;
    }

    if (hnm_terminal_command_matches(command, "ls")) {
        hnm_terminal_execute_ls(hnm_terminal_command_argument(command, "ls"));
        return;
    }

    if (hnm_terminal_command_matches(command, "cat")) {
        hnm_terminal_execute_cat(hnm_terminal_command_argument(command, "cat"));
        return;
    }

    if (hnm_string_equal(command, "version")) {
        hnm_log_write_line("terminal command: version");
        hnm_console_write_line("HNMos v0.4 keyboard terminal");
        hnm_terminal_prompt();
        return;
    }

    if (hnm_string_equal(command, "clear")) {
        hnm_log_write_line("terminal command: clear");
        hnm_console_clear();
        hnm_terminal_prompt();
        return;
    }

    if (hnm_string_equal(command, "back") ||
        hnm_string_equal(command, "exit") ||
        hnm_string_equal(command, "launcher")) {
        hnm_terminal_return_to_launcher(command);
        return;
    }

    if (hnm_string_equal(command, "halt")) {
        hnm_log_write_line("terminal command: halt");
        hnm_console_write_line("halting");
        hnm_kernel_halt();
    }

    if (hnm_string_equal(command, "reboot")) {
        hnm_terminal_reboot();
        return;
    }

    hnm_log_write_line("terminal command: unknown");
    hnm_console_write("unknown command: ");
    hnm_console_write_line(command);
    hnm_terminal_prompt();
}

static void hnm_terminal_submit_line(void)
{
    hnm_terminal_input[hnm_terminal_input_length] = '\0';
    hnm_console_newline();
    hnm_terminal_execute_command(hnm_terminal_input);
    hnm_terminal_input_length = 0;
}

static void hnm_terminal_handle_char(char character)
{
    if (hnm_terminal_input_length + 1 >= HNM_TERMINAL_INPUT_MAX) {
        return;
    }

    hnm_terminal_input[hnm_terminal_input_length] = character;
    hnm_terminal_input_length++;
    hnm_console_put_char(character);
}

static void hnm_terminal_handle_backspace(void)
{
    if (hnm_terminal_input_length == 0) {
        return;
    }

    hnm_terminal_input_length--;
    hnm_console_backspace();
}

void hnm_terminal_init(void)
{
    hnm_terminal_fg = hnm_color_rgb(232, 238, 245);
    hnm_terminal_bg = hnm_color_rgb(6, 10, 16);
    hnm_terminal_accent = hnm_color_rgb(67, 220, 210);
    hnm_terminal_input_length = 0;
    hnm_console_set_colors(hnm_terminal_fg, hnm_terminal_bg);
}

void hnm_terminal_show_screen(void)
{
    hnm_terminal_input_length = 0;
    hnm_console_set_colors(hnm_terminal_fg, hnm_terminal_bg);
    hnm_console_clear();
    hnm_console_set_cursor(4, 4);
    hnm_console_write_line("HNMos Terminal");
    hnm_console_write_line("commands: help version clear halt reboot ls cat tasks ai back exit launcher");
    hnm_console_write_line("ai commands: ai help, ai sysinfo, ai status");
    hnm_console_write_line("ESC/back/exit/launcher returns to Launcher");
    hnm_console_newline();
    hnm_terminal_prompt();
}

void hnm_terminal_handle_event(const struct hnm_ui_event *event)
{
    if (event->type == HNM_UI_EVENT_KEY_CHAR) {
        hnm_terminal_handle_char(event->character);
    } else if (event->type == HNM_UI_EVENT_KEY_DOWN && event->key_code == HNM_UI_KEY_BACKSPACE) {
        hnm_terminal_handle_backspace();
    } else if (event->type == HNM_UI_EVENT_KEY_DOWN && event->key_code == HNM_UI_KEY_ENTER) {
        hnm_terminal_submit_line();
    }
}

void hnm_terminal_poll(void)
{
    hnm_input_dispatcher_dispatch(hnm_terminal_handle_event);
}

#include "mouse.h"
#include "../interrupts/interrupts.h"
#include "../kernel/io.h"
#include "../kernel/log.h"
#include "../kernel/types.h"
#include "../kernel/graphics/cursor.h"
#include "../kernel/graphics/framebuffer.h"
#include "../kernel/interface/events.h"

enum {
    HNM_PS2_DATA_PORT = 0x60,
    HNM_PS2_STATUS_PORT = 0x64,
    HNM_PS2_COMMAND_PORT = 0x64,
    HNM_PS2_STATUS_OUTPUT_FULL = 1 << 0,
    HNM_PS2_STATUS_INPUT_FULL = 1 << 1,
    HNM_PS2_STATUS_MOUSE_DATA = 1 << 5,
    HNM_PS2_COMMAND_READ_CONFIG = 0x20,
    HNM_PS2_COMMAND_WRITE_CONFIG = 0x60,
    HNM_PS2_COMMAND_ENABLE_MOUSE_PORT = 0xA8,
    HNM_PS2_COMMAND_WRITE_MOUSE = 0xD4,
    HNM_PS2_CONFIG_MOUSE_IRQ = 1 << 1,
    HNM_PS2_CONFIG_MOUSE_CLOCK_DISABLED = 1 << 5,
    HNM_MOUSE_COMMAND_SET_DEFAULTS = 0xF6,
    HNM_MOUSE_COMMAND_ENABLE_DATA = 0xF4,
    HNM_MOUSE_ACK = 0xFA,
    HNM_MOUSE_IRQ = 12,
    HNM_MOUSE_PACKET_SIZE = 3,
    HNM_MOUSE_WAIT_ATTEMPTS = 100000
};

static u8 hnm_mouse_packet[HNM_MOUSE_PACKET_SIZE];
static volatile u32 hnm_mouse_packet_index;
static volatile u32 hnm_mouse_x;
static volatile u32 hnm_mouse_y;
static volatile int hnm_mouse_left_button;
static volatile int hnm_mouse_right_button;
static volatile int hnm_mouse_middle_button;
static volatile int hnm_mouse_ready;
static volatile int hnm_mouse_dirty;

static int hnm_ps2_wait_input_clear(void)
{
    for (u32 attempt = 0; attempt < HNM_MOUSE_WAIT_ATTEMPTS; attempt++) {
        if ((hnm_inb(HNM_PS2_STATUS_PORT) & HNM_PS2_STATUS_INPUT_FULL) == 0) {
            return 1;
        }
    }

    return 0;
}

static int hnm_ps2_wait_output_full(void)
{
    for (u32 attempt = 0; attempt < HNM_MOUSE_WAIT_ATTEMPTS; attempt++) {
        if ((hnm_inb(HNM_PS2_STATUS_PORT) & HNM_PS2_STATUS_OUTPUT_FULL) != 0) {
            return 1;
        }
    }

    return 0;
}

static void hnm_ps2_flush_output(void)
{
    for (u32 attempt = 0; attempt < 64; attempt++) {
        if ((hnm_inb(HNM_PS2_STATUS_PORT) & HNM_PS2_STATUS_OUTPUT_FULL) == 0) {
            return;
        }

        (void)hnm_inb(HNM_PS2_DATA_PORT);
    }
}

static int hnm_ps2_write_controller_command(u8 command)
{
    if (!hnm_ps2_wait_input_clear()) {
        return 0;
    }

    hnm_outb(HNM_PS2_COMMAND_PORT, command);
    return 1;
}

static int hnm_ps2_read_config(u8 *config)
{
    if (!hnm_ps2_write_controller_command(HNM_PS2_COMMAND_READ_CONFIG)) {
        return 0;
    }

    if (!hnm_ps2_wait_output_full()) {
        return 0;
    }

    *config = hnm_inb(HNM_PS2_DATA_PORT);
    return 1;
}

static int hnm_ps2_write_config(u8 config)
{
    if (!hnm_ps2_write_controller_command(HNM_PS2_COMMAND_WRITE_CONFIG)) {
        return 0;
    }

    if (!hnm_ps2_wait_input_clear()) {
        return 0;
    }

    hnm_outb(HNM_PS2_DATA_PORT, config);
    return 1;
}

static int hnm_mouse_write_command(u8 command)
{
    u8 response;

    if (!hnm_ps2_write_controller_command(HNM_PS2_COMMAND_WRITE_MOUSE)) {
        return 0;
    }

    if (!hnm_ps2_wait_input_clear()) {
        return 0;
    }

    hnm_outb(HNM_PS2_DATA_PORT, command);

    if (!hnm_ps2_wait_output_full()) {
        return 0;
    }

    response = hnm_inb(HNM_PS2_DATA_PORT);
    return response == HNM_MOUSE_ACK;
}

static int hnm_mouse_fail(const char *message)
{
    hnm_mouse_ready = 0;
    hnm_mouse_dirty = 0;
    hnm_log_write_line(message);
    return 0;
}

static int hnm_mouse_sign_extend(u8 value)
{
    if ((value & 0x80) != 0) {
        return (int)value - 256;
    }

    return (int)value;
}

static u8 hnm_mouse_buttons_mask(int left_button, int right_button, int middle_button)
{
    u8 buttons = 0;

    if (left_button) {
        buttons |= HNM_UI_MOUSE_LEFT_MASK;
    }

    if (right_button) {
        buttons |= HNM_UI_MOUSE_RIGHT_MASK;
    }

    if (middle_button) {
        buttons |= HNM_UI_MOUSE_MIDDLE_MASK;
    }

    return buttons;
}

static void hnm_mouse_push_move_event(int dx, int dy, u8 buttons)
{
    struct hnm_ui_event event;

    if (dx == 0 && dy == 0) {
        return;
    }

    hnm_ui_event_init(&event, HNM_UI_EVENT_MOUSE_MOVE);
    event.x = hnm_mouse_x;
    event.y = hnm_mouse_y;
    event.dx = dx;
    event.dy = dy;
    event.buttons = buttons;
    hnm_ui_event_push(&event);
}

static void hnm_mouse_push_button_event(enum hnm_ui_event_type type, u8 button, u8 buttons)
{
    struct hnm_ui_event event;

    hnm_ui_event_init(&event, type);
    event.x = hnm_mouse_x;
    event.y = hnm_mouse_y;
    event.button = button;
    event.buttons = buttons;
    hnm_ui_event_push(&event);
}

static void hnm_mouse_push_button_changes(
    int old_left_button,
    int old_right_button,
    int old_middle_button,
    int new_left_button,
    int new_right_button,
    int new_middle_button,
    u8 buttons)
{
    if (old_left_button != new_left_button) {
        hnm_mouse_push_button_event(
            new_left_button ? HNM_UI_EVENT_MOUSE_BUTTON_DOWN : HNM_UI_EVENT_MOUSE_BUTTON_UP,
            HNM_UI_MOUSE_BUTTON_LEFT,
            buttons);
    }

    if (old_right_button != new_right_button) {
        hnm_mouse_push_button_event(
            new_right_button ? HNM_UI_EVENT_MOUSE_BUTTON_DOWN : HNM_UI_EVENT_MOUSE_BUTTON_UP,
            HNM_UI_MOUSE_BUTTON_RIGHT,
            buttons);
    }

    if (old_middle_button != new_middle_button) {
        hnm_mouse_push_button_event(
            new_middle_button ? HNM_UI_EVENT_MOUSE_BUTTON_DOWN : HNM_UI_EVENT_MOUSE_BUTTON_UP,
            HNM_UI_MOUSE_BUTTON_MIDDLE,
            buttons);
    }
}

static void hnm_mouse_clamp_position(int *x, int *y)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    int max_x;
    int max_y;

    if (!hnm_framebuffer_is_available() || fb->width == 0 || fb->height == 0) {
        *x = 0;
        *y = 0;
        return;
    }

    max_x = (int)fb->width - 1;
    max_y = (int)fb->height - 1;

    if (*x < 0) {
        *x = 0;
    } else if (*x > max_x) {
        *x = max_x;
    }

    if (*y < 0) {
        *y = 0;
    } else if (*y > max_y) {
        *y = max_y;
    }
}

static void hnm_mouse_apply_packet(void)
{
    int dx;
    int dy;
    int next_x;
    int next_y;
    int old_left_button;
    int old_right_button;
    int old_middle_button;
    int new_left_button;
    int new_right_button;
    int new_middle_button;
    u8 buttons;

    if ((hnm_mouse_packet[0] & 0xC0) != 0) {
        return;
    }

    old_left_button = hnm_mouse_left_button;
    old_right_button = hnm_mouse_right_button;
    old_middle_button = hnm_mouse_middle_button;
    new_left_button = (hnm_mouse_packet[0] & 0x01) != 0;
    new_right_button = (hnm_mouse_packet[0] & 0x02) != 0;
    new_middle_button = (hnm_mouse_packet[0] & 0x04) != 0;
    dx = hnm_mouse_sign_extend(hnm_mouse_packet[1]);
    dy = hnm_mouse_sign_extend(hnm_mouse_packet[2]);
    next_x = (int)hnm_mouse_x + dx;
    next_y = (int)hnm_mouse_y - dy;
    hnm_mouse_clamp_position(&next_x, &next_y);

    hnm_mouse_x = (u32)next_x;
    hnm_mouse_y = (u32)next_y;
    hnm_mouse_left_button = new_left_button;
    hnm_mouse_right_button = new_right_button;
    hnm_mouse_middle_button = new_middle_button;
    buttons = hnm_mouse_buttons_mask(new_left_button, new_right_button, new_middle_button);
    hnm_mouse_push_move_event(dx, -dy, buttons);
    hnm_mouse_push_button_changes(
        old_left_button,
        old_right_button,
        old_middle_button,
        new_left_button,
        new_right_button,
        new_middle_button,
        buttons);
    hnm_mouse_dirty = 1;
}

static void hnm_mouse_handle_byte(u8 value)
{
    if (hnm_mouse_packet_index == 0 && (value & 0x08) == 0) {
        return;
    }

    hnm_mouse_packet[hnm_mouse_packet_index] = value;
    hnm_mouse_packet_index++;

    if (hnm_mouse_packet_index < HNM_MOUSE_PACKET_SIZE) {
        return;
    }

    hnm_mouse_packet_index = 0;
    hnm_mouse_apply_packet();
}

int hnm_mouse_init(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u8 config;

    hnm_mouse_ready = 0;
    hnm_mouse_dirty = 0;
    hnm_mouse_packet_index = 0;
    hnm_mouse_left_button = 0;
    hnm_mouse_right_button = 0;
    hnm_mouse_middle_button = 0;

    if (!hnm_framebuffer_is_available()) {
        return hnm_mouse_fail("mouse: framebuffer unavailable; cursor disabled.");
    }

    hnm_mouse_x = fb->width / 2;
    hnm_mouse_y = fb->height / 2;

    hnm_ps2_flush_output();

    if (!hnm_ps2_write_controller_command(HNM_PS2_COMMAND_ENABLE_MOUSE_PORT)) {
        return hnm_mouse_fail("mouse: PS/2 controller did not accept enable command.");
    }

    if (!hnm_ps2_read_config(&config)) {
        return hnm_mouse_fail("mouse: PS/2 controller config read failed.");
    }

    config |= HNM_PS2_CONFIG_MOUSE_IRQ;
    config &= (u8)~HNM_PS2_CONFIG_MOUSE_CLOCK_DISABLED;

    if (!hnm_ps2_write_config(config)) {
        return hnm_mouse_fail("mouse: PS/2 controller config write failed.");
    }

    if (!hnm_mouse_write_command(HNM_MOUSE_COMMAND_SET_DEFAULTS)) {
        return hnm_mouse_fail("mouse: PS/2 set defaults command failed.");
    }

    if (!hnm_mouse_write_command(HNM_MOUSE_COMMAND_ENABLE_DATA)) {
        return hnm_mouse_fail("mouse: PS/2 data reporting command failed.");
    }

    hnm_mouse_ready = 1;
    hnm_cursor_init(hnm_mouse_x, hnm_mouse_y);
    hnm_log_write_line("mouse: PS/2 IRQ12 ready.");
    return 1;
}

int hnm_mouse_is_ready(void)
{
    return hnm_mouse_ready;
}

void hnm_mouse_irq_handler(void)
{
    u8 status = hnm_inb(HNM_PS2_STATUS_PORT);

    if ((status & HNM_PS2_STATUS_OUTPUT_FULL) != 0 &&
        (status & HNM_PS2_STATUS_MOUSE_DATA) != 0) {
        u8 value = hnm_inb(HNM_PS2_DATA_PORT);

        if (hnm_mouse_ready) {
            hnm_mouse_handle_byte(value);
        }
    }

    hnm_pic_send_eoi(HNM_MOUSE_IRQ);
}

void hnm_mouse_poll(void)
{
    u32 x;
    u32 y;
    int left_button;
    int right_button;
    int middle_button;

    if (!hnm_mouse_ready || !hnm_mouse_dirty) {
        return;
    }

    x = hnm_mouse_x;
    y = hnm_mouse_y;
    left_button = hnm_mouse_left_button;
    right_button = hnm_mouse_right_button;
    middle_button = hnm_mouse_middle_button;
    hnm_mouse_dirty = 0;

    hnm_cursor_move(x, y, left_button, right_button, middle_button);
}

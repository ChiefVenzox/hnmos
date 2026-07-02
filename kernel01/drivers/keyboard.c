#include "keyboard.h"
#include "../interrupts/interrupts.h"
#include "../kernel/io.h"
#include "../kernel/log.h"
#include "../kernel/interface/events.h"
#include "../kernel/types.h"

enum {
    HNM_KEYBOARD_DATA_PORT = 0x60,
    HNM_KEYBOARD_STATUS_PORT = 0x64,
    HNM_KEYBOARD_STATUS_OUTPUT_FULL = 1 << 0,
    HNM_KEYBOARD_IRQ = 1
};

static char hnm_keyboard_scancode_to_char(u8 scancode)
{
    static const char keymap[128] = {
        [0x02] = '1',
        [0x03] = '2',
        [0x04] = '3',
        [0x05] = '4',
        [0x06] = '5',
        [0x07] = '6',
        [0x08] = '7',
        [0x09] = '8',
        [0x0A] = '9',
        [0x0B] = '0',
        [0x0C] = '-',
        [0x10] = 'q',
        [0x11] = 'w',
        [0x12] = 'e',
        [0x13] = 'r',
        [0x14] = 't',
        [0x15] = 'y',
        [0x16] = 'u',
        [0x17] = 'i',
        [0x18] = 'o',
        [0x19] = 'p',
        [0x1E] = 'a',
        [0x1F] = 's',
        [0x20] = 'd',
        [0x21] = 'f',
        [0x22] = 'g',
        [0x23] = 'h',
        [0x24] = 'j',
        [0x25] = 'k',
        [0x26] = 'l',
        [0x2C] = 'z',
        [0x2D] = 'x',
        [0x2E] = 'c',
        [0x2F] = 'v',
        [0x30] = 'b',
        [0x31] = 'n',
        [0x32] = 'm',
        [0x34] = '.',
        [0x35] = '/',
        [0x39] = ' '
    };

    return keymap[scancode];
}

static void hnm_keyboard_push_key_down(u8 scancode, u32 key_code, char character)
{
    struct hnm_ui_event event;

    hnm_ui_event_init(&event, HNM_UI_EVENT_KEY_DOWN);
    event.scancode = scancode;
    event.key_code = key_code;
    event.character = character;
    hnm_ui_event_push(&event);
}

static void hnm_keyboard_push_key_char(u8 scancode, char character)
{
    struct hnm_ui_event event;

    hnm_ui_event_init(&event, HNM_UI_EVENT_KEY_CHAR);
    event.scancode = scancode;
    event.key_code = (u32)character;
    event.character = character;
    hnm_ui_event_push(&event);
}

static void hnm_keyboard_handle_scancode(u8 scancode)
{
    char character;

    if ((scancode & 0x80) != 0) {
        return;
    }

    if (scancode == 0x1C) {
        hnm_keyboard_push_key_down(scancode, HNM_UI_KEY_ENTER, '\n');
        return;
    }

    if (scancode == 0x0E) {
        hnm_keyboard_push_key_down(scancode, HNM_UI_KEY_BACKSPACE, '\b');
        return;
    }

    if (scancode == 0x01) {
        hnm_keyboard_push_key_down(scancode, HNM_UI_KEY_ESCAPE, '\0');
        return;
    }

    if (scancode == 0x3B) {
        hnm_keyboard_push_key_down(scancode, HNM_UI_KEY_F1, '\0');
        return;
    }

    character = hnm_keyboard_scancode_to_char(scancode);

    if (character != '\0') {
        hnm_keyboard_push_key_down(scancode, (u32)character, character);
        hnm_keyboard_push_key_char(scancode, character);
    }
}

void hnm_keyboard_init(void)
{
    while ((hnm_inb(HNM_KEYBOARD_STATUS_PORT) & HNM_KEYBOARD_STATUS_OUTPUT_FULL) != 0) {
        (void)hnm_inb(HNM_KEYBOARD_DATA_PORT);
    }

    hnm_log_write_line("keyboard: PS/2 scancode set 1 ready.");
}

void hnm_keyboard_irq_handler(void)
{
    if ((hnm_inb(HNM_KEYBOARD_STATUS_PORT) & HNM_KEYBOARD_STATUS_OUTPUT_FULL) != 0) {
        hnm_keyboard_handle_scancode(hnm_inb(HNM_KEYBOARD_DATA_PORT));
    }

    hnm_pic_send_eoi(HNM_KEYBOARD_IRQ);
}

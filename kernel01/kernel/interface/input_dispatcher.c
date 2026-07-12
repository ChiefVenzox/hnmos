#include "input_dispatcher.h"
#include "../ai/ai_session.h"
#include "../graphics/color.h"
#include "../graphics/font.h"
#include "../graphics/framebuffer.h"
#include "../graphics/primitive.h"
#include "../log.h"

enum {
    HNM_INPUT_STATUS_WIDTH = 320,
    HNM_INPUT_STATUS_HEIGHT = 20,
    HNM_INPUT_STATUS_MARGIN = 8
};

static void hnm_input_append_char(char *text, u32 *index, u32 capacity, char value)
{
    if (*index + 1 >= capacity) {
        return;
    }

    text[*index] = value;
    *index = *index + 1;
}

static void hnm_input_append_text(char *text, u32 *index, u32 capacity, const char *value)
{
    for (u32 i = 0; value[i] != '\0'; i++) {
        hnm_input_append_char(text, index, capacity, value[i]);
    }
}

static void hnm_input_append_u32(char *text, u32 *index, u32 capacity, u32 value)
{
    char digits[10];
    u32 digit_count = 0;

    if (value == 0) {
        hnm_input_append_char(text, index, capacity, '0');
        return;
    }

    while (value != 0 && digit_count < 10) {
        digits[digit_count] = (char)('0' + (value % 10));
        value /= 10;
        digit_count++;
    }

    while (digit_count > 0) {
        digit_count--;
        hnm_input_append_char(text, index, capacity, digits[digit_count]);
    }
}

static void hnm_input_append_button(char *text, u32 *index, u32 capacity, u8 button)
{
    if (button == HNM_UI_MOUSE_BUTTON_LEFT) {
        hnm_input_append_text(text, index, capacity, " left");
    } else if (button == HNM_UI_MOUSE_BUTTON_RIGHT) {
        hnm_input_append_text(text, index, capacity, " right");
    } else if (button == HNM_UI_MOUSE_BUTTON_MIDDLE) {
        hnm_input_append_text(text, index, capacity, " middle");
    }
}

static void hnm_input_event_text(const struct hnm_ui_event *event, char *text, u32 capacity)
{
    u32 index = 0;

    if (event->type == HNM_UI_EVENT_KEY_CHAR) {
        hnm_input_append_text(text, &index, capacity, "event key char ");

        if (event->character == ' ') {
            hnm_input_append_text(text, &index, capacity, "space");
        } else {
            hnm_input_append_char(text, &index, capacity, event->character);
        }
    } else if (event->type == HNM_UI_EVENT_KEY_DOWN) {
        hnm_input_append_text(text, &index, capacity, "event key down");
    } else if (event->type == HNM_UI_EVENT_MOUSE_MOVE) {
        hnm_input_append_text(text, &index, capacity, "event mouse move ");
        hnm_input_append_u32(text, &index, capacity, event->x);
        hnm_input_append_text(text, &index, capacity, " ");
        hnm_input_append_u32(text, &index, capacity, event->y);
    } else if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_DOWN) {
        hnm_input_append_text(text, &index, capacity, "event button down");
        hnm_input_append_button(text, &index, capacity, event->button);
    } else if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_UP) {
        hnm_input_append_text(text, &index, capacity, "event button up");
        hnm_input_append_button(text, &index, capacity, event->button);
    } else {
        hnm_input_append_text(text, &index, capacity, "event none");
    }

    text[index] = '\0';
}

static void hnm_input_draw_recent_event(const struct hnm_ui_event *event)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    hnm_color background = hnm_color_rgb(15, 24, 36);
    hnm_color border = hnm_color_rgb(132, 122, 255);
    hnm_color text_color = hnm_color_rgb(232, 238, 245);
    char text[64];
    u32 panel_x;
    u32 panel_y = 36;
    u32 text_x;
    u32 text_y = 42;

    if (!hnm_framebuffer_is_available()) {
        return;
    }

    panel_x = fb->width > HNM_INPUT_STATUS_WIDTH + (HNM_INPUT_STATUS_MARGIN * 2) ?
        fb->width - HNM_INPUT_STATUS_WIDTH - HNM_INPUT_STATUS_MARGIN :
        HNM_INPUT_STATUS_MARGIN;
    text_x = panel_x + 8;

    hnm_input_event_text(event, text, sizeof(text));
    hnm_draw_fill_rect(panel_x, panel_y, HNM_INPUT_STATUS_WIDTH, HNM_INPUT_STATUS_HEIGHT, background);
    hnm_draw_rect(panel_x, panel_y, HNM_INPUT_STATUS_WIDTH, HNM_INPUT_STATUS_HEIGHT, border);
    hnm_font_draw_string(text_x, text_y, text, text_color, background);
}

void hnm_input_dispatcher_init(void)
{
    hnm_log_write_line("input dispatcher: event polling ready.");
}

void hnm_input_dispatcher_dispatch(hnm_input_event_handler handler)
{
    struct hnm_ui_event event;

    while (hnm_ui_event_poll(&event)) {
        if (!hnm_ai_session_is_capturing_secret()) {
            hnm_input_draw_recent_event(&event);
        }

        if (handler != 0) {
            handler(&event);
        }
    }
}

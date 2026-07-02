#include "events.h"
#include "../log.h"

enum {
    HNM_UI_EVENT_QUEUE_SIZE = 128
};

static struct hnm_ui_event hnm_ui_event_queue[HNM_UI_EVENT_QUEUE_SIZE];
static volatile u32 hnm_ui_event_head;
static volatile u32 hnm_ui_event_tail;
static volatile u32 hnm_ui_event_sequence;
static volatile u32 hnm_ui_event_dropped;
static struct hnm_ui_event hnm_ui_event_last;
static int hnm_ui_event_has_recent;

void hnm_ui_event_init(struct hnm_ui_event *event, enum hnm_ui_event_type type)
{
    event->type = type;
    event->sequence = 0;
    event->x = 0;
    event->y = 0;
    event->dx = 0;
    event->dy = 0;
    event->key_code = HNM_UI_KEY_NONE;
    event->scancode = 0;
    event->button = HNM_UI_MOUSE_BUTTON_NONE;
    event->buttons = 0;
    event->character = '\0';
}

void hnm_ui_events_init(void)
{
    hnm_ui_event_head = 0;
    hnm_ui_event_tail = 0;
    hnm_ui_event_sequence = 0;
    hnm_ui_event_dropped = 0;
    hnm_ui_event_has_recent = 0;
    hnm_ui_event_init(&hnm_ui_event_last, HNM_UI_EVENT_NONE);
    hnm_log_write_line("ui events: fixed queue ready.");
}

int hnm_ui_event_push(const struct hnm_ui_event *event)
{
    u32 next_head = (hnm_ui_event_head + 1) % HNM_UI_EVENT_QUEUE_SIZE;
    struct hnm_ui_event queued_event;

    if (next_head == hnm_ui_event_tail) {
        hnm_ui_event_dropped++;
        return 0;
    }

    queued_event = *event;
    queued_event.sequence = hnm_ui_event_sequence;
    hnm_ui_event_sequence++;

    hnm_ui_event_queue[hnm_ui_event_head] = queued_event;
    hnm_ui_event_head = next_head;
    hnm_ui_event_last = queued_event;
    hnm_ui_event_has_recent = 1;
    return 1;
}

int hnm_ui_event_poll(struct hnm_ui_event *event)
{
    if (hnm_ui_event_tail == hnm_ui_event_head) {
        return 0;
    }

    *event = hnm_ui_event_queue[hnm_ui_event_tail];
    hnm_ui_event_tail = (hnm_ui_event_tail + 1) % HNM_UI_EVENT_QUEUE_SIZE;
    return 1;
}

int hnm_ui_event_recent(struct hnm_ui_event *event)
{
    if (!hnm_ui_event_has_recent) {
        return 0;
    }

    *event = hnm_ui_event_last;
    return 1;
}

u32 hnm_ui_event_pending_count(void)
{
    if (hnm_ui_event_head >= hnm_ui_event_tail) {
        return hnm_ui_event_head - hnm_ui_event_tail;
    }

    return (HNM_UI_EVENT_QUEUE_SIZE - hnm_ui_event_tail) + hnm_ui_event_head;
}

u32 hnm_ui_event_dropped_count(void)
{
    return hnm_ui_event_dropped;
}

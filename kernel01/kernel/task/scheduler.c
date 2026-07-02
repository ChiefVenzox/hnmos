#include "scheduler.h"
#include "../log.h"

static int hnm_scheduler_ready;
static u32 hnm_scheduler_current_index;
static u32 hnm_scheduler_yield_total;

static void hnm_idle_task(void *argument)
{
    (void)argument;
}

static void hnm_ui_task(void *argument)
{
    (void)argument;
}

static int hnm_scheduler_select_task(u32 start_index)
{
    u32 count = hnm_task_count();

    if (count == 0) {
        return 0;
    }

    for (u32 offset = 1; offset <= count; offset++) {
        u32 index = (start_index + offset) % count;
        const struct hnm_task *candidate = hnm_task_get(index);

        if (candidate != 0 && candidate->state == HNM_TASK_READY) {
            hnm_scheduler_current_index = index;
            hnm_task_set_state(candidate->id, HNM_TASK_RUNNING);
            hnm_task_increment_yield_count(candidate->id);
            hnm_scheduler_yield_total++;
            return 1;
        }
    }

    return 0;
}

void hnm_scheduler_init(void)
{
    u32 ui_task_id = 0;

    hnm_task_registry_init();
    hnm_scheduler_ready = 1;
    hnm_scheduler_current_index = 0;
    hnm_scheduler_yield_total = 0;

    (void)hnm_scheduler_add_task("idle", hnm_idle_task, 0, 0);
    ui_task_id = (u32)hnm_scheduler_add_task("ui", hnm_ui_task, 0, 0);

    if (ui_task_id != 0 && hnm_task_index_by_id(ui_task_id, &hnm_scheduler_current_index)) {
        hnm_task_set_state(ui_task_id, HNM_TASK_RUNNING);
    }

    hnm_log_write_line("scheduler: cooperative skeleton ready.");
    hnm_log_write_line("scheduler: internal tasks registered.");
}

int hnm_scheduler_add_task(const char *name, hnm_task_entry entry, void *argument, void *stack_pointer)
{
    u32 task_id = 0;

    if (!hnm_task_register(name, entry, argument, stack_pointer, HNM_TASK_READY, &task_id)) {
        return 0;
    }

    return (int)task_id;
}

void hnm_scheduler_yield(void)
{
    const struct hnm_task *current;

    if (!hnm_scheduler_ready || hnm_task_count() == 0) {
        return;
    }

    current = hnm_task_get(hnm_scheduler_current_index);

    if (current != 0 && current->state == HNM_TASK_RUNNING) {
        hnm_task_set_state(current->id, HNM_TASK_READY);
    }

    if (!hnm_scheduler_select_task(hnm_scheduler_current_index) && current != 0) {
        hnm_task_set_state(current->id, HNM_TASK_RUNNING);
    }
}

void hnm_scheduler_get_stats(struct hnm_scheduler_stats *stats)
{
    const struct hnm_task *current;

    if (stats == 0) {
        return;
    }

    current = hnm_scheduler_current_task();
    stats->ready = hnm_scheduler_ready;
    stats->task_count = hnm_task_count();
    stats->current_task_id = current != 0 ? current->id : 0;
    stats->yield_count = hnm_scheduler_yield_total;
}

const struct hnm_task *hnm_scheduler_current_task(void)
{
    if (!hnm_scheduler_ready) {
        return 0;
    }

    return hnm_task_get(hnm_scheduler_current_index);
}

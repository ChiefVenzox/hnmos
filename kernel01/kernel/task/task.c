#include "task.h"
#include "../log.h"

static struct hnm_task hnm_tasks[HNM_TASK_MAX];
static u32 hnm_task_total;
static u32 hnm_task_next_id;

void hnm_task_registry_init(void)
{
    hnm_task_total = 0;
    hnm_task_next_id = 1;
    hnm_log_write_line("tasks: registry ready.");
}

int hnm_task_register(
    const char *name,
    hnm_task_entry entry,
    void *argument,
    void *stack_pointer,
    enum hnm_task_state initial_state,
    u32 *task_id)
{
    struct hnm_task *task;

    if (name == 0 || hnm_task_total >= HNM_TASK_MAX) {
        return 0;
    }

    task = &hnm_tasks[hnm_task_total];
    task->id = hnm_task_next_id;
    task->name = name;
    task->state = initial_state;
    task->entry = entry;
    task->argument = argument;
    hnm_task_context_init(&task->context, stack_pointer);
    task->yield_count = 0;

    if (task_id != 0) {
        *task_id = task->id;
    }

    hnm_task_total++;
    hnm_task_next_id++;
    return 1;
}

u32 hnm_task_count(void)
{
    return hnm_task_total;
}

const struct hnm_task *hnm_task_get(u32 index)
{
    if (index >= hnm_task_total) {
        return 0;
    }

    return &hnm_tasks[index];
}

const struct hnm_task *hnm_task_get_by_id(u32 task_id)
{
    for (u32 i = 0; i < hnm_task_total; i++) {
        if (hnm_tasks[i].id == task_id) {
            return &hnm_tasks[i];
        }
    }

    return 0;
}

int hnm_task_index_by_id(u32 task_id, u32 *index)
{
    for (u32 i = 0; i < hnm_task_total; i++) {
        if (hnm_tasks[i].id == task_id) {
            if (index != 0) {
                *index = i;
            }

            return 1;
        }
    }

    return 0;
}

int hnm_task_set_state(u32 task_id, enum hnm_task_state state)
{
    for (u32 i = 0; i < hnm_task_total; i++) {
        if (hnm_tasks[i].id == task_id) {
            hnm_tasks[i].state = state;
            return 1;
        }
    }

    return 0;
}

int hnm_task_increment_yield_count(u32 task_id)
{
    for (u32 i = 0; i < hnm_task_total; i++) {
        if (hnm_tasks[i].id == task_id) {
            hnm_tasks[i].yield_count++;
            return 1;
        }
    }

    return 0;
}

const char *hnm_task_state_name(enum hnm_task_state state)
{
    if (state == HNM_TASK_READY) {
        return "ready";
    }

    if (state == HNM_TASK_RUNNING) {
        return "running";
    }

    if (state == HNM_TASK_BLOCKED) {
        return "blocked";
    }

    if (state == HNM_TASK_STOPPED) {
        return "stopped";
    }

    return "unknown";
}

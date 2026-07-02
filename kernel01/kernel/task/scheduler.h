#ifndef HNM_TASK_SCHEDULER_H
#define HNM_TASK_SCHEDULER_H

#include "../types.h"
#include "task.h"

struct hnm_scheduler_stats {
    int ready;
    u32 task_count;
    u32 current_task_id;
    u32 yield_count;
};

void hnm_scheduler_init(void);
int hnm_scheduler_add_task(const char *name, hnm_task_entry entry, void *argument, void *stack_pointer);
void hnm_scheduler_yield(void);
void hnm_scheduler_get_stats(struct hnm_scheduler_stats *stats);
const struct hnm_task *hnm_scheduler_current_task(void);

#endif

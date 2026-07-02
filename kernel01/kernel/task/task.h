#ifndef HNM_TASK_TASK_H
#define HNM_TASK_TASK_H

#include "../types.h"
#include "context.h"

enum hnm_task_state {
    HNM_TASK_READY = 1,
    HNM_TASK_RUNNING = 2,
    HNM_TASK_BLOCKED = 3,
    HNM_TASK_STOPPED = 4
};

typedef void (*hnm_task_entry)(void *argument);

struct hnm_task {
    u32 id;
    const char *name;
    enum hnm_task_state state;
    hnm_task_entry entry;
    void *argument;
    struct hnm_task_context context;
    u32 yield_count;
};

enum {
    HNM_TASK_MAX = 8
};

void hnm_task_registry_init(void);
int hnm_task_register(
    const char *name,
    hnm_task_entry entry,
    void *argument,
    void *stack_pointer,
    enum hnm_task_state initial_state,
    u32 *task_id);
u32 hnm_task_count(void);
const struct hnm_task *hnm_task_get(u32 index);
const struct hnm_task *hnm_task_get_by_id(u32 task_id);
int hnm_task_index_by_id(u32 task_id, u32 *index);
int hnm_task_set_state(u32 task_id, enum hnm_task_state state);
int hnm_task_increment_yield_count(u32 task_id);
const char *hnm_task_state_name(enum hnm_task_state state);

#endif

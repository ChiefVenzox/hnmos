#ifndef HNM_AI_TASK_H
#define HNM_AI_TASK_H

#include "../types.h"

enum {
    HNM_AI_TASK_INPUT_MAX = 160,
    HNM_AI_TASK_OUTPUT_MAX = 192
};

enum hnm_ai_task_type {
    HNM_AI_TASK_NONE = 0,
    HNM_AI_TASK_HELP,
    HNM_AI_TASK_SYSINFO,
    HNM_AI_TASK_STATUS,
    HNM_AI_TASK_ASSEMBLY,
    HNM_AI_TASK_SYNC,
    HNM_AI_TASK_HNLANG,
    HNM_AI_TASK_GENERIC
};

enum hnm_ai_task_status {
    AI_TASK_EMPTY = 0,
    AI_TASK_PENDING,
    AI_TASK_RUNNING,
    AI_TASK_DONE,
    AI_TASK_ERROR
};

struct hnm_ai_task {
    u32 task_id;
    enum hnm_ai_task_type task_type;
    char input[HNM_AI_TASK_INPUT_MAX];
    u32 context_flags;
    enum hnm_ai_task_status status;
    char output[HNM_AI_TASK_OUTPUT_MAX];
};

void hnm_ai_task_init(struct hnm_ai_task *task);
void hnm_ai_task_prepare(
    struct hnm_ai_task *task,
    u32 task_id,
    enum hnm_ai_task_type task_type,
    const char *input,
    u32 context_flags);
void hnm_ai_task_set_output(struct hnm_ai_task *task, const char *output);
const char *hnm_ai_task_status_name(enum hnm_ai_task_status status);
const char *hnm_ai_task_type_name(enum hnm_ai_task_type task_type);

#endif

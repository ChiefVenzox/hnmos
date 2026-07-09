#include "ai_task.h"

static void hnm_ai_copy_text(char *dest, u32 capacity, const char *source)
{
    u32 index = 0;

    if (capacity == 0) {
        return;
    }

    if (source != 0) {
        while (source[index] != '\0' && index + 1 < capacity) {
            dest[index] = source[index];
            index++;
        }
    }

    dest[index] = '\0';
}

void hnm_ai_task_init(struct hnm_ai_task *task)
{
    if (task == 0) {
        return;
    }

    task->task_id = 0;
    task->task_type = HNM_AI_TASK_NONE;
    task->input[0] = '\0';
    task->context_flags = 0;
    task->status = AI_TASK_EMPTY;
    task->output[0] = '\0';
}

void hnm_ai_task_prepare(
    struct hnm_ai_task *task,
    u32 task_id,
    enum hnm_ai_task_type task_type,
    const char *input,
    u32 context_flags)
{
    if (task == 0) {
        return;
    }

    task->task_id = task_id;
    task->task_type = task_type;
    hnm_ai_copy_text(task->input, sizeof(task->input), input);
    task->context_flags = context_flags;
    task->status = AI_TASK_PENDING;
    task->output[0] = '\0';
}

void hnm_ai_task_set_output(struct hnm_ai_task *task, const char *output)
{
    if (task == 0) {
        return;
    }

    hnm_ai_copy_text(task->output, sizeof(task->output), output);
}

const char *hnm_ai_task_status_name(enum hnm_ai_task_status status)
{
    if (status == AI_TASK_EMPTY) {
        return "empty";
    }

    if (status == AI_TASK_PENDING) {
        return "pending";
    }

    if (status == AI_TASK_RUNNING) {
        return "running";
    }

    if (status == AI_TASK_DONE) {
        return "done";
    }

    if (status == AI_TASK_ERROR) {
        return "error";
    }

    return "unknown";
}

const char *hnm_ai_task_type_name(enum hnm_ai_task_type task_type)
{
    if (task_type == HNM_AI_TASK_HELP) {
        return "help";
    }

    if (task_type == HNM_AI_TASK_SYSINFO) {
        return "sysinfo";
    }

    if (task_type == HNM_AI_TASK_STATUS) {
        return "status";
    }

    if (task_type == HNM_AI_TASK_ASSEMBLY) {
        return "assembly";
    }

    if (task_type == HNM_AI_TASK_SYNC) {
        return "sync";
    }

    if (task_type == HNM_AI_TASK_HNLANG) {
        return "hnlang";
    }

    if (task_type == HNM_AI_TASK_GENERIC) {
        return "generic";
    }

    return "none";
}

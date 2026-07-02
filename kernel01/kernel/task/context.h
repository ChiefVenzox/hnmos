#ifndef HNM_TASK_CONTEXT_H
#define HNM_TASK_CONTEXT_H

struct hnm_task_context {
    void *stack_pointer;
};

void hnm_task_context_init(struct hnm_task_context *context, void *stack_pointer);
void *hnm_task_context_stack_pointer(const struct hnm_task_context *context);

#endif

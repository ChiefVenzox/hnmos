#include "context.h"

void hnm_task_context_init(struct hnm_task_context *context, void *stack_pointer)
{
    if (context == 0) {
        return;
    }

    context->stack_pointer = stack_pointer;
}

void *hnm_task_context_stack_pointer(const struct hnm_task_context *context)
{
    if (context == 0) {
        return 0;
    }

    return context->stack_pointer;
}

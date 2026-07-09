#include "ai_stub_provider.h"

static int hnm_ai_stub_submit_task(struct hnm_ai_task *task)
{
    if (task == 0 || task->status == AI_TASK_EMPTY) {
        return 0;
    }

    task->status = AI_TASK_PENDING;
    return 1;
}

static int hnm_ai_stub_poll_task(struct hnm_ai_task *task)
{
    if (task == 0 || task->status == AI_TASK_EMPTY) {
        return 0;
    }

    task->status = AI_TASK_RUNNING;

    if (task->task_type == HNM_AI_TASK_HELP) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: Future HNMos assistant will help with system tasks, scripts, and diagnostics.");
    } else if (task->task_type == HNM_AI_TASK_SYSINFO) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: Kernel status summary is available. Real AI runtime is not enabled yet.");
    } else if (task->task_type == HNM_AI_TASK_STATUS) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: provider active. Real AI runtime is not enabled yet.");
    } else if (task->task_type == HNM_AI_TASK_ASSEMBLY) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: AI assembly plan ready. Kernel snapshot observed; external actions remain gated.");
    } else if (task->task_type == HNM_AI_TASK_SYNC) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: Kernel-owned CPU/RAM/framebuffer checkpoint executed; external actions remain gated.");
    } else if (task->task_type == HNM_AI_TASK_HNLANG) {
        hnm_ai_task_set_output(
            task,
            "AI Stub: HNlang AI task accepted as a placeholder. No model executed.");
    } else {
        hnm_ai_task_set_output(
            task,
            "AI Stub: deterministic placeholder response. Real AI runtime is not enabled yet.");
    }

    task->status = AI_TASK_DONE;
    return 1;
}

static const char *hnm_ai_stub_get_result(const struct hnm_ai_task *task)
{
    if (task == 0) {
        return "AI Stub: no task available.";
    }

    if (task->status != AI_TASK_DONE) {
        return "AI Stub: task is not complete.";
    }

    return task->output;
}

static const struct hnm_ai_provider hnm_ai_stub = {
    "HNMos Stub Provider",
    HNM_AI_PROVIDER_STUB,
    hnm_ai_stub_submit_task,
    hnm_ai_stub_poll_task,
    hnm_ai_stub_get_result
};

const struct hnm_ai_provider *hnm_ai_stub_provider(void)
{
    return &hnm_ai_stub;
}

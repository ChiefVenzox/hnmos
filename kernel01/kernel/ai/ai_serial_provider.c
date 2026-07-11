#include "ai_serial_provider.h"
#include "ai_serial_transport.h"

static int hnm_ai_serial_submit_task(struct hnm_ai_task *task)
{
    if (task == 0 || task->status == AI_TASK_EMPTY) {
        return 0;
    }

    if (task->task_type == HNM_AI_TASK_HNLANG && task->language_profile == 0) {
        task->status = AI_TASK_ERROR;
        hnm_ai_task_set_output(task, "HNLang profile missing; request denied.");
        return 0;
    }

    if (!hnm_ai_serial_transport_send(task)) {
        task->status = AI_TASK_ERROR;
        hnm_ai_task_set_output(task, "AI transport send failed.");
        return 0;
    }

    task->status = AI_TASK_RUNNING;
    return 1;
}

static int hnm_ai_serial_poll_task(struct hnm_ai_task *task)
{
    if (task == 0 || task->status != AI_TASK_RUNNING) {
        return 0;
    }

    return hnm_ai_serial_transport_poll(task);
}

static const char *hnm_ai_serial_get_result(const struct hnm_ai_task *task)
{
    if (task == 0) {
        return "AI bridge: no task.";
    }

    if (task->status == AI_TASK_RUNNING || task->status == AI_TASK_PENDING) {
        return "AI bridge: waiting for provider response.";
    }

    if (task->output[0] == '\0') {
        return "AI bridge: provider returned no output.";
    }

    return task->output;
}

static const struct hnm_ai_provider hnm_ai_serial = {
    "HNMos OpenAI Serial Bridge",
    HNM_AI_PROVIDER_EXTERNAL_BRIDGE,
    hnm_ai_serial_submit_task,
    hnm_ai_serial_poll_task,
    hnm_ai_serial_get_result
};

const struct hnm_ai_provider *hnm_ai_serial_provider(void)
{
    hnm_ai_serial_transport_init();
    return &hnm_ai_serial;
}

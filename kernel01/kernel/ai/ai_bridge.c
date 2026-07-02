#include "ai_bridge.h"
#include "ai_context.h"
#include "ai_stub_provider.h"
#include "../log.h"

static const struct hnm_ai_provider *hnm_ai_active_provider;
static struct hnm_ai_task hnm_ai_last_task;
static u32 hnm_ai_next_task_id;
static int hnm_ai_bridge_ready;

void hnm_ai_bridge_init(void)
{
    hnm_ai_active_provider = hnm_ai_stub_provider();
    hnm_ai_task_init(&hnm_ai_last_task);
    hnm_ai_next_task_id = 1;
    hnm_ai_bridge_ready = hnm_ai_active_provider != 0;

    if (hnm_ai_bridge_ready) {
        hnm_log_write("ai bridge: provider active: ");
        hnm_log_write_line(hnm_ai_active_provider->provider_name);
    } else {
        hnm_log_write_line("ai bridge: no provider active.");
    }
}

int hnm_ai_bridge_submit(enum hnm_ai_task_type task_type, const char *input, u32 context_flags)
{
    if (!hnm_ai_bridge_ready ||
        hnm_ai_active_provider == 0 ||
        hnm_ai_active_provider->submit_task == 0) {
        return 0;
    }

    hnm_ai_task_prepare(
        &hnm_ai_last_task,
        hnm_ai_next_task_id,
        task_type,
        input,
        context_flags);
    hnm_ai_next_task_id++;

    hnm_log_write("ai bridge: submit ");
    hnm_log_write(hnm_ai_task_type_name(task_type));
    hnm_log_write(" context ");
    hnm_log_write_line(hnm_ai_context_summary(context_flags));

    return hnm_ai_active_provider->submit_task(&hnm_ai_last_task);
}

int hnm_ai_bridge_poll(void)
{
    if (!hnm_ai_bridge_ready ||
        hnm_ai_active_provider == 0 ||
        hnm_ai_active_provider->poll_task == 0) {
        return 0;
    }

    return hnm_ai_active_provider->poll_task(&hnm_ai_last_task);
}

const char *hnm_ai_bridge_result(void)
{
    if (!hnm_ai_bridge_ready ||
        hnm_ai_active_provider == 0 ||
        hnm_ai_active_provider->get_result == 0) {
        return "AI Stub: no provider active. Real AI runtime is not enabled yet.";
    }

    return hnm_ai_active_provider->get_result(&hnm_ai_last_task);
}

const char *hnm_ai_bridge_provider_name(void)
{
    if (!hnm_ai_bridge_ready || hnm_ai_active_provider == 0) {
        return "none";
    }

    return hnm_ai_active_provider->provider_name;
}

const char *hnm_ai_bridge_provider_type_name(void)
{
    if (!hnm_ai_bridge_ready || hnm_ai_active_provider == 0) {
        return hnm_ai_provider_type_name(HNM_AI_PROVIDER_NONE);
    }

    return hnm_ai_provider_type_name(hnm_ai_active_provider->provider_type);
}

const char *hnm_ai_bridge_status_text(void)
{
    if (!hnm_ai_bridge_ready) {
        return "AI Stub: bridge inactive. Real AI runtime is not enabled yet.";
    }

    return "AI Stub: provider active. Real AI runtime is not enabled yet.";
}

const struct hnm_ai_task *hnm_ai_bridge_last_task(void)
{
    return &hnm_ai_last_task;
}

const char *hnm_ai_bridge_request_help(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_HELP, "ai help", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI Stub: submit failed. Real AI runtime is not enabled yet.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_sysinfo(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_SYSINFO, "ai sysinfo", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI Stub: submit failed. Real AI runtime is not enabled yet.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_status(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_STATUS, "ai status", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI Stub: submit failed. Real AI runtime is not enabled yet.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

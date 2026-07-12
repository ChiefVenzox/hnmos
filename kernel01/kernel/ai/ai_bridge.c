#include "ai_bridge.h"
#include "ai_context.h"
#include "ai_serial_provider.h"
#include "ai_stub_provider.h"
#include "hnlang_ai_profile.h"
#include "../log.h"

static const struct hnm_ai_provider *hnm_ai_active_provider;
static const struct hnm_ai_provider *hnm_ai_local_provider;
static const struct hnm_ai_provider *hnm_ai_last_provider;
static struct hnm_ai_task hnm_ai_last_task;
static struct hnm_ai_assembly_program hnm_ai_last_assembly_program;
static u32 hnm_ai_next_task_id;
static int hnm_ai_bridge_ready;

void hnm_ai_bridge_init(void)
{
    hnm_ai_active_provider = hnm_ai_serial_provider();
    hnm_ai_local_provider = hnm_ai_stub_provider();
    hnm_ai_last_provider = hnm_ai_local_provider;
    hnm_ai_task_init(&hnm_ai_last_task);
    hnm_ai_assembly_init(&hnm_ai_last_assembly_program);
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
    const struct hnm_ai_provider *provider = task_type == HNM_AI_TASK_HNLANG ?
        hnm_ai_active_provider : hnm_ai_local_provider;

    if (!hnm_ai_bridge_ready || provider == 0 || provider->submit_task == 0) {
        return 0;
    }

    hnm_ai_task_prepare(
        &hnm_ai_last_task,
        hnm_ai_next_task_id,
        task_type,
        input,
        context_flags);

    if (task_type == HNM_AI_TASK_HNLANG) {
        hnm_ai_last_task.language_profile = hnm_hnlang_ai_system_contract();
    }

    hnm_ai_next_task_id++;
    hnm_ai_last_provider = provider;
    hnm_ai_assembly_build(&hnm_ai_last_assembly_program, &hnm_ai_last_task);

    hnm_log_write("ai bridge: submit ");
    hnm_log_write(hnm_ai_task_type_name(task_type));
    hnm_log_write(" context ");
    hnm_log_write_line(hnm_ai_context_summary(context_flags));
    hnm_log_write("ai bridge: ");
    hnm_log_write_line(hnm_ai_assembly_machine_summary(&hnm_ai_last_assembly_program));

    return hnm_ai_last_provider->submit_task(&hnm_ai_last_task);
}

int hnm_ai_bridge_poll(void)
{
    if (!hnm_ai_bridge_ready ||
        hnm_ai_last_provider == 0 ||
        hnm_ai_last_provider->poll_task == 0) {
        return 0;
    }

    return hnm_ai_last_provider->poll_task(&hnm_ai_last_task);
}

int hnm_ai_bridge_cancel(void)
{
    if (hnm_ai_last_task.status != AI_TASK_RUNNING &&
        hnm_ai_last_task.status != AI_TASK_PENDING) {
        return 0;
    }

    hnm_ai_last_task.status = AI_TASK_CANCELLED;
    hnm_ai_task_set_output(&hnm_ai_last_task, "AI request cancelled locally.");
    return 1;
}

const char *hnm_ai_bridge_result(void)
{
    if (!hnm_ai_bridge_ready ||
        hnm_ai_last_provider == 0 ||
        hnm_ai_last_provider->get_result == 0) {
        return "AI bridge: no provider active.";
    }

    return hnm_ai_last_provider->get_result(&hnm_ai_last_task);
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
        return "AI bridge: inactive.";
    }

    return "AI bridge: external provider transport active.";
}

const struct hnm_ai_task *hnm_ai_bridge_last_task(void)
{
    return &hnm_ai_last_task;
}

const struct hnm_ai_assembly_program *hnm_ai_bridge_last_assembly_program(void)
{
    return &hnm_ai_last_assembly_program;
}

const char *hnm_ai_bridge_machine_summary(void)
{
    hnm_ai_machine_capture(&hnm_ai_last_assembly_program.machine);
    return hnm_ai_assembly_machine_summary(&hnm_ai_last_assembly_program);
}

const char *hnm_ai_bridge_sync_summary(void)
{
    hnm_sync_capture(&hnm_ai_last_assembly_program.sync);
    return hnm_ai_assembly_sync_summary(&hnm_ai_last_assembly_program);
}

const char *hnm_ai_bridge_request_help(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_HELP, "ai help", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI bridge: help request submit failed.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_assembly(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_ASSEMBLY, "ai asm", HNM_AI_CONTEXT_MACHINE_STATE)) {
        return "AI bridge: assembly request submit failed.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_sync(void)
{
    if (!hnm_ai_bridge_submit(
            HNM_AI_TASK_SYNC,
            "ai sync",
            HNM_AI_CONTEXT_MACHINE_STATE | HNM_AI_CONTEXT_SYNC_STATE)) {
        return "AI bridge: sync request submit failed.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_sysinfo(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_SYSINFO, "ai sysinfo", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI bridge: sysinfo request submit failed.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

const char *hnm_ai_bridge_request_status(void)
{
    if (!hnm_ai_bridge_submit(HNM_AI_TASK_STATUS, "ai status", HNM_AI_CONTEXT_SYSTEM_INFO)) {
        return "AI bridge: status request submit failed.";
    }

    hnm_ai_bridge_poll();
    return hnm_ai_bridge_result();
}

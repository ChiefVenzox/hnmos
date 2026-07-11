#ifndef HNM_AI_BRIDGE_H
#define HNM_AI_BRIDGE_H

#include "ai_assembly.h"
#include "ai_provider.h"
#include "ai_task.h"
#include "../types.h"

void hnm_ai_bridge_init(void);
int hnm_ai_bridge_submit(enum hnm_ai_task_type task_type, const char *input, u32 context_flags);
int hnm_ai_bridge_poll(void);
int hnm_ai_bridge_cancel(void);
const char *hnm_ai_bridge_result(void);
const char *hnm_ai_bridge_provider_name(void);
const char *hnm_ai_bridge_provider_type_name(void);
const char *hnm_ai_bridge_status_text(void);
const struct hnm_ai_task *hnm_ai_bridge_last_task(void);
const struct hnm_ai_assembly_program *hnm_ai_bridge_last_assembly_program(void);
const char *hnm_ai_bridge_machine_summary(void);
const char *hnm_ai_bridge_sync_summary(void);
const char *hnm_ai_bridge_request_help(void);
const char *hnm_ai_bridge_request_sysinfo(void);
const char *hnm_ai_bridge_request_status(void);
const char *hnm_ai_bridge_request_assembly(void);
const char *hnm_ai_bridge_request_sync(void);

#endif

#ifndef HNM_AI_ASSEMBLY_H
#define HNM_AI_ASSEMBLY_H

#include "ai_machine.h"
#include "ai_task.h"
#include "../sync/system_sync.h"
#include "../types.h"

enum {
    HNM_AI_ASM_INSTRUCTION_MAX = 6
};

enum hnm_ai_assembly_opcode {
    HNM_AI_ASM_NOP = 0,
    HNM_AI_ASM_OBSERVE_MACHINE,
    HNM_AI_ASM_ROUTE_CONTEXT,
    HNM_AI_ASM_BOUND_TASK,
    HNM_AI_ASM_DRAFT_RESPONSE,
    HNM_AI_ASM_WAIT_PERMISSION,
    HNM_AI_ASM_SYNC_CPU,
    HNM_AI_ASM_SYNC_RAM,
    HNM_AI_ASM_SYNC_GPU,
    HNM_AI_ASM_RETURN_RESULT
};

struct hnm_ai_assembly_instruction {
    enum hnm_ai_assembly_opcode opcode;
    u32 operand;
};

struct hnm_ai_assembly_program {
    u32 instruction_count;
    enum hnm_ai_task_type task_type;
    u32 context_flags;
    struct hnm_ai_machine_snapshot machine;
    struct hnm_sync_snapshot sync;
    struct hnm_ai_assembly_instruction instructions[HNM_AI_ASM_INSTRUCTION_MAX];
};

void hnm_ai_assembly_init(struct hnm_ai_assembly_program *program);
void hnm_ai_assembly_build(
    struct hnm_ai_assembly_program *program,
    const struct hnm_ai_task *task);
const char *hnm_ai_assembly_opcode_name(enum hnm_ai_assembly_opcode opcode);
const char *hnm_ai_assembly_machine_summary(const struct hnm_ai_assembly_program *program);
const char *hnm_ai_assembly_sync_summary(const struct hnm_ai_assembly_program *program);

#endif

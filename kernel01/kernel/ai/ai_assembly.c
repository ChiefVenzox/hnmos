#include "ai_assembly.h"

static void hnm_ai_assembly_append(
    struct hnm_ai_assembly_program *program,
    enum hnm_ai_assembly_opcode opcode,
    u32 operand)
{
    u32 index;

    if (program == 0 ||
        program->instruction_count >= HNM_AI_ASM_INSTRUCTION_MAX) {
        return;
    }

    index = program->instruction_count;
    program->instructions[index].opcode = opcode;
    program->instructions[index].operand = operand;
    program->instruction_count++;
}

void hnm_ai_assembly_init(struct hnm_ai_assembly_program *program)
{
    if (program == 0) {
        return;
    }

    program->instruction_count = 0;
    program->task_type = HNM_AI_TASK_NONE;
    program->context_flags = 0;
    hnm_ai_machine_capture(&program->machine);
    hnm_sync_capture(&program->sync);

    for (u32 i = 0; i < HNM_AI_ASM_INSTRUCTION_MAX; i++) {
        program->instructions[i].opcode = HNM_AI_ASM_NOP;
        program->instructions[i].operand = 0;
    }
}

void hnm_ai_assembly_build(
    struct hnm_ai_assembly_program *program,
    const struct hnm_ai_task *task)
{
    if (program == 0) {
        return;
    }

    hnm_ai_assembly_init(program);

    if (task == 0) {
        return;
    }

    program->task_type = task->task_type;
    program->context_flags = task->context_flags;

    if (task->task_type == HNM_AI_TASK_SYNC) {
        hnm_sync_checkpoint(&program->sync);
        hnm_ai_assembly_append(program, HNM_AI_ASM_OBSERVE_MACHINE, program->machine.state_flags);
        hnm_ai_assembly_append(program, HNM_AI_ASM_SYNC_CPU, program->sync.cpu_fence_count);
        hnm_ai_assembly_append(program, HNM_AI_ASM_SYNC_RAM, program->sync.ram_fence_count);
        hnm_ai_assembly_append(program, HNM_AI_ASM_SYNC_GPU, program->sync.gpu_fence_count);
        hnm_ai_assembly_append(program, HNM_AI_ASM_ROUTE_CONTEXT, task->context_flags);
        hnm_ai_assembly_append(program, HNM_AI_ASM_RETURN_RESULT, 0);
        return;
    }

    hnm_ai_assembly_append(program, HNM_AI_ASM_OBSERVE_MACHINE, program->machine.state_flags);
    hnm_ai_assembly_append(program, HNM_AI_ASM_ROUTE_CONTEXT, task->context_flags);
    hnm_ai_assembly_append(program, HNM_AI_ASM_BOUND_TASK, task->task_type);
    hnm_ai_assembly_append(program, HNM_AI_ASM_DRAFT_RESPONSE, 0);
    hnm_ai_assembly_append(program, HNM_AI_ASM_WAIT_PERMISSION, 0);
    hnm_ai_assembly_append(program, HNM_AI_ASM_RETURN_RESULT, 0);
}

const char *hnm_ai_assembly_opcode_name(enum hnm_ai_assembly_opcode opcode)
{
    if (opcode == HNM_AI_ASM_OBSERVE_MACHINE) {
        return "observe_machine";
    }

    if (opcode == HNM_AI_ASM_ROUTE_CONTEXT) {
        return "route_context";
    }

    if (opcode == HNM_AI_ASM_BOUND_TASK) {
        return "bound_task";
    }

    if (opcode == HNM_AI_ASM_DRAFT_RESPONSE) {
        return "draft_response";
    }

    if (opcode == HNM_AI_ASM_WAIT_PERMISSION) {
        return "wait_permission";
    }

    if (opcode == HNM_AI_ASM_SYNC_CPU) {
        return "sync_cpu";
    }

    if (opcode == HNM_AI_ASM_SYNC_RAM) {
        return "sync_ram";
    }

    if (opcode == HNM_AI_ASM_SYNC_GPU) {
        return "sync_gpu";
    }

    if (opcode == HNM_AI_ASM_RETURN_RESULT) {
        return "return_result";
    }

    return "nop";
}

const char *hnm_ai_assembly_machine_summary(const struct hnm_ai_assembly_program *program)
{
    if (program == 0) {
        return "machine: unavailable";
    }

    return hnm_ai_machine_summary(&program->machine);
}

const char *hnm_ai_assembly_sync_summary(const struct hnm_ai_assembly_program *program)
{
    if (program == 0) {
        return "sync: unavailable";
    }

    return hnm_sync_summary(&program->sync);
}

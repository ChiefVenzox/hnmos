#include "ai_machine.h"
#include "ai_machine_asm.h"

enum {
    HNM_AI_CR0_PE = 0x00000001,
    HNM_AI_EFLAGS_IF = 0x00000200
};

#define HNM_AI_CR0_PG 0x80000000u

void hnm_ai_machine_capture(struct hnm_ai_machine_snapshot *snapshot)
{
    u32 cr0;
    u32 eflags;

    if (snapshot == 0) {
        return;
    }

    cr0 = hnm_ai_machine_read_cr0();
    eflags = hnm_ai_machine_read_eflags();

    snapshot->state_flags = 0;
    snapshot->cr0_safe_bits = cr0 & (HNM_AI_CR0_PE | HNM_AI_CR0_PG);
    snapshot->eflags_safe_bits = eflags & HNM_AI_EFLAGS_IF;

    if ((cr0 & HNM_AI_CR0_PE) != 0) {
        snapshot->state_flags |= HNM_AI_MACHINE_PROTECTED_MODE;
    }

    if ((cr0 & HNM_AI_CR0_PG) != 0) {
        snapshot->state_flags |= HNM_AI_MACHINE_PAGING_ENABLED;
    }

    if ((eflags & HNM_AI_EFLAGS_IF) != 0) {
        snapshot->state_flags |= HNM_AI_MACHINE_INTERRUPTS_ENABLED;
    }
}

int hnm_ai_machine_has_flag(const struct hnm_ai_machine_snapshot *snapshot, u32 flag)
{
    if (snapshot == 0) {
        return 0;
    }

    return (snapshot->state_flags & flag) != 0;
}

const char *hnm_ai_machine_summary(const struct hnm_ai_machine_snapshot *snapshot)
{
    int protected_mode;
    int paging_enabled;
    int interrupts_enabled;

    if (snapshot == 0) {
        return "machine: unavailable";
    }

    protected_mode = hnm_ai_machine_has_flag(snapshot, HNM_AI_MACHINE_PROTECTED_MODE);
    paging_enabled = hnm_ai_machine_has_flag(snapshot, HNM_AI_MACHINE_PAGING_ENABLED);
    interrupts_enabled = hnm_ai_machine_has_flag(snapshot, HNM_AI_MACHINE_INTERRUPTS_ENABLED);

    if (protected_mode && paging_enabled && interrupts_enabled) {
        return "machine: protected on, paging on, irq on";
    }

    if (protected_mode && paging_enabled) {
        return "machine: protected on, paging on, irq off";
    }

    if (protected_mode && interrupts_enabled) {
        return "machine: protected on, paging off, irq on";
    }

    if (protected_mode) {
        return "machine: protected on, paging off, irq off";
    }

    if (paging_enabled && interrupts_enabled) {
        return "machine: protected off, paging on, irq on";
    }

    if (paging_enabled) {
        return "machine: protected off, paging on, irq off";
    }

    if (interrupts_enabled) {
        return "machine: protected off, paging off, irq on";
    }

    return "machine: protected off, paging off, irq off";
}

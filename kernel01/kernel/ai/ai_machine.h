#ifndef HNM_AI_MACHINE_H
#define HNM_AI_MACHINE_H

#include "../types.h"

enum hnm_ai_machine_flags {
    HNM_AI_MACHINE_PROTECTED_MODE = 1 << 0,
    HNM_AI_MACHINE_PAGING_ENABLED = 1 << 1,
    HNM_AI_MACHINE_INTERRUPTS_ENABLED = 1 << 2
};

struct hnm_ai_machine_snapshot {
    u32 state_flags;
    u32 cr0_safe_bits;
    u32 eflags_safe_bits;
};

void hnm_ai_machine_capture(struct hnm_ai_machine_snapshot *snapshot);
int hnm_ai_machine_has_flag(const struct hnm_ai_machine_snapshot *snapshot, u32 flag);
const char *hnm_ai_machine_summary(const struct hnm_ai_machine_snapshot *snapshot);

#endif

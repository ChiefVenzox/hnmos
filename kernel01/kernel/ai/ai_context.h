#ifndef HNM_AI_CONTEXT_H
#define HNM_AI_CONTEXT_H

#include "../types.h"

enum hnm_ai_context_flags {
    HNM_AI_CONTEXT_NONE = 0,
    HNM_AI_CONTEXT_SYSTEM_INFO = 1 << 0,
    HNM_AI_CONTEXT_MEMORY_INFO = 1 << 1,
    HNM_AI_CONTEXT_GRAPHICS_INFO = 1 << 2,
    HNM_AI_CONTEXT_INPUT_INFO = 1 << 3,
    HNM_AI_CONTEXT_HNLANG_TASK = 1 << 4,
    HNM_AI_CONTEXT_MACHINE_STATE = 1 << 5,
    HNM_AI_CONTEXT_SYNC_STATE = 1 << 6
};

const char *hnm_ai_context_summary(u32 context_flags);

#endif

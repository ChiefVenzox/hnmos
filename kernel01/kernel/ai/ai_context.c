#include "ai_context.h"

const char *hnm_ai_context_summary(u32 context_flags)
{
    if (context_flags == HNM_AI_CONTEXT_NONE) {
        return "none";
    }

    if ((context_flags & HNM_AI_CONTEXT_HNLANG_TASK) != 0) {
        return "hnlang task";
    }

    if ((context_flags & HNM_AI_CONTEXT_SYSTEM_INFO) != 0) {
        return "system info";
    }

    if ((context_flags & HNM_AI_CONTEXT_MEMORY_INFO) != 0) {
        return "memory info";
    }

    if ((context_flags & HNM_AI_CONTEXT_GRAPHICS_INFO) != 0) {
        return "graphics info";
    }

    if ((context_flags & HNM_AI_CONTEXT_INPUT_INFO) != 0) {
        return "input info";
    }

    return "mixed";
}

#ifndef HNM_AI_PROVIDER_H
#define HNM_AI_PROVIDER_H

#include "ai_task.h"

enum hnm_ai_provider_type {
    HNM_AI_PROVIDER_NONE = 0,
    HNM_AI_PROVIDER_STUB,
    HNM_AI_PROVIDER_LOCAL_TINY_MODEL,
    HNM_AI_PROVIDER_EXTERNAL_BRIDGE,
    HNM_AI_PROVIDER_HARDWARE_ACCELERATOR,
    HNM_AI_PROVIDER_OFFLINE_RULE_ENGINE
};

struct hnm_ai_provider {
    const char *provider_name;
    enum hnm_ai_provider_type provider_type;
    int (*submit_task)(struct hnm_ai_task *task);
    int (*poll_task)(struct hnm_ai_task *task);
    const char *(*get_result)(const struct hnm_ai_task *task);
};

const char *hnm_ai_provider_type_name(enum hnm_ai_provider_type provider_type);

#endif

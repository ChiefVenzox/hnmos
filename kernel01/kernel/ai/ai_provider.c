#include "ai_provider.h"

const char *hnm_ai_provider_type_name(enum hnm_ai_provider_type provider_type)
{
    if (provider_type == HNM_AI_PROVIDER_STUB) {
        return "stub";
    }

    if (provider_type == HNM_AI_PROVIDER_LOCAL_TINY_MODEL) {
        return "local_tiny_model";
    }

    if (provider_type == HNM_AI_PROVIDER_EXTERNAL_BRIDGE) {
        return "external_bridge";
    }

    if (provider_type == HNM_AI_PROVIDER_HARDWARE_ACCELERATOR) {
        return "hardware_accelerator";
    }

    if (provider_type == HNM_AI_PROVIDER_OFFLINE_RULE_ENGINE) {
        return "offline_rule_engine";
    }

    return "none";
}

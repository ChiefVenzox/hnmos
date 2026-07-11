#ifndef HNM_AI_HNLANG_PROFILE_H
#define HNM_AI_HNLANG_PROFILE_H

#include "../types.h"

struct hnm_hnlang_ai_primitive {
    const char *name;
    const char *input_type;
    const char *output_type;
    const char *effect;
};

struct hnm_hnlang_ai_recipe {
    const char *name;
    const char *combination;
    u32 maximum_steps;
};

const char *hnm_hnlang_ai_profile_summary(void);
const char *hnm_hnlang_ai_system_contract(void);
u32 hnm_hnlang_ai_primitive_count(void);
const struct hnm_hnlang_ai_primitive *hnm_hnlang_ai_primitive_at(u32 index);
u32 hnm_hnlang_ai_recipe_count(void);
const struct hnm_hnlang_ai_recipe *hnm_hnlang_ai_recipe_at(u32 index);

#endif

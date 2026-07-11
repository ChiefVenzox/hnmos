#include "hnlang_ai_profile.h"

static const struct hnm_hnlang_ai_primitive hnm_hnlang_ai_primitives[] = {
    {"intent.plan", "UserIntent", "TaskPlan", "pure"},
    {"code.generate", "TaskPlan", "HNLangDraft", "workspace-draft"},
    {"code.refactor", "HNLangDraft", "HNLangDraft", "workspace-draft"},
    {"policy.validate", "HNLangDraft", "PolicyReport", "pure"},
    {"build.request", "HNLangDraft", "ApprovalRequest", "approval"},
    {"test.explain", "BuildReport", "Explanation", "read-only"},
    {"preview.render", "HNLangDraft", "StudioPreview", "pure"}
};

static const struct hnm_hnlang_ai_recipe hnm_hnlang_ai_recipes[] = {
    {
        "app.create",
        "intent.plan > code.generate > policy.validate > preview.render",
        4
    },
    {
        "app.optimize",
        "code.refactor > policy.validate > build.request > test.explain",
        4
    },
    {
        "error.repair",
        "intent.plan > code.refactor > policy.validate > build.request > test.explain",
        5
    },
    {
        "ui.compose",
        "intent.plan > code.generate > policy.validate > preview.render",
        4
    }
};

const char *hnm_hnlang_ai_profile_summary(void)
{
    return "HNLang AI profile: 7 typed primitives + 4 bounded recipes";
}

const char *hnm_hnlang_ai_system_contract(void)
{
    return
        "HNLang v0 is HNMos' typed task/UI language. Grammar: print STRING; let ID = EXPR; "
        "route STRING; ai task STRING { input EXPR mode STRING output EXPR }; app STRING { "
        "title STRING screen STRING { panel STRING { text EXPR } button STRING { route STRING "
        "} } }. Read-only names: kernel.version, kernel.ticks, kernel.memory.total, "
        "graphics.width, graphics.height, system.info. Return plain HNLang for /workspace "
        "only; never modify kernel, boot, policy, drivers, or audit.";
}

u32 hnm_hnlang_ai_primitive_count(void)
{
    return sizeof(hnm_hnlang_ai_primitives) / sizeof(hnm_hnlang_ai_primitives[0]);
}

const struct hnm_hnlang_ai_primitive *hnm_hnlang_ai_primitive_at(u32 index)
{
    if (index >= hnm_hnlang_ai_primitive_count()) {
        return 0;
    }

    return &hnm_hnlang_ai_primitives[index];
}

u32 hnm_hnlang_ai_recipe_count(void)
{
    return sizeof(hnm_hnlang_ai_recipes) / sizeof(hnm_hnlang_ai_recipes[0]);
}

const struct hnm_hnlang_ai_recipe *hnm_hnlang_ai_recipe_at(u32 index)
{
    if (index >= hnm_hnlang_ai_recipe_count()) {
        return 0;
    }

    return &hnm_hnlang_ai_recipes[index];
}

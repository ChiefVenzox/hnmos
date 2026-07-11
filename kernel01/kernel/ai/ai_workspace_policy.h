#ifndef HNM_AI_WORKSPACE_POLICY_H
#define HNM_AI_WORKSPACE_POLICY_H

#include "../types.h"

enum hnm_ai_workspace_capability {
    HNM_AI_CAP_READ_WORKSPACE = 0,
    HNM_AI_CAP_WRITE_HNLANG_DRAFT,
    HNM_AI_CAP_READ_BUILD_LOG,
    HNM_AI_CAP_RUN_WORKSPACE_BUILD,
    HNM_AI_CAP_USE_PROVIDER_NETWORK,
    HNM_AI_CAP_MODIFY_SYSTEM,
    HNM_AI_CAP_WRITE_KERNEL,
    HNM_AI_CAP_WRITE_BOOT,
    HNM_AI_CAP_CHANGE_POLICY,
    HNM_AI_CAP_CONTROL_DRIVER,
    HNM_AI_CAP_CHANGE_AUDIT_LOG
};

enum hnm_ai_policy_decision {
    HNM_AI_POLICY_DENY = 0,
    HNM_AI_POLICY_ALLOW,
    HNM_AI_POLICY_REQUIRE_APPROVAL
};

enum hnm_ai_policy_decision hnm_ai_workspace_policy_decide(
    enum hnm_ai_workspace_capability capability);
enum hnm_ai_policy_decision hnm_ai_workspace_policy_decide_path(
    enum hnm_ai_workspace_capability capability,
    const char *path);
const char *hnm_ai_workspace_capability_name(
    enum hnm_ai_workspace_capability capability);
const char *hnm_ai_policy_decision_name(enum hnm_ai_policy_decision decision);

#endif

#include "ai_workspace_policy.h"

static int hnm_ai_policy_has_prefix(const char *text, const char *prefix)
{
    u32 index = 0;

    if (text == 0 || prefix == 0) {
        return 0;
    }

    while (prefix[index] != '\0') {
        if (text[index] != prefix[index]) {
            return 0;
        }

        index++;
    }

    return 1;
}

static int hnm_ai_policy_contains_parent_reference(const char *path)
{
    if (path == 0) {
        return 1;
    }

    for (u32 i = 0; path[i] != '\0'; i++) {
        if (path[i] == '.' && path[i + 1] == '.') {
            return 1;
        }
    }

    return 0;
}

static int hnm_ai_policy_has_hnlang_suffix(const char *path)
{
    u32 length = 0;

    if (path == 0) {
        return 0;
    }

    while (path[length] != '\0') {
        length++;
    }

    return length >= 3 &&
        path[length - 3] == '.' &&
        path[length - 2] == 'h' &&
        path[length - 1] == 'n';
}

enum hnm_ai_policy_decision hnm_ai_workspace_policy_decide(
    enum hnm_ai_workspace_capability capability)
{
    if (capability == HNM_AI_CAP_READ_WORKSPACE ||
        capability == HNM_AI_CAP_WRITE_HNLANG_DRAFT ||
        capability == HNM_AI_CAP_READ_BUILD_LOG) {
        return HNM_AI_POLICY_ALLOW;
    }

    if (capability == HNM_AI_CAP_RUN_WORKSPACE_BUILD ||
        capability == HNM_AI_CAP_USE_PROVIDER_NETWORK) {
        return HNM_AI_POLICY_REQUIRE_APPROVAL;
    }

    return HNM_AI_POLICY_DENY;
}

enum hnm_ai_policy_decision hnm_ai_workspace_policy_decide_path(
    enum hnm_ai_workspace_capability capability,
    const char *path)
{
    enum hnm_ai_policy_decision base_decision =
        hnm_ai_workspace_policy_decide(capability);

    if (base_decision != HNM_AI_POLICY_ALLOW) {
        return base_decision;
    }

    if (!hnm_ai_policy_has_prefix(path, "/workspace/") ||
        hnm_ai_policy_contains_parent_reference(path)) {
        return HNM_AI_POLICY_DENY;
    }

    if (capability == HNM_AI_CAP_WRITE_HNLANG_DRAFT &&
        !hnm_ai_policy_has_hnlang_suffix(path)) {
        return HNM_AI_POLICY_DENY;
    }

    return HNM_AI_POLICY_ALLOW;
}

const char *hnm_ai_workspace_capability_name(
    enum hnm_ai_workspace_capability capability)
{
    if (capability == HNM_AI_CAP_READ_WORKSPACE) {
        return "read.workspace";
    }

    if (capability == HNM_AI_CAP_WRITE_HNLANG_DRAFT) {
        return "write.workspace.hnlang";
    }

    if (capability == HNM_AI_CAP_READ_BUILD_LOG) {
        return "read.build.log";
    }

    if (capability == HNM_AI_CAP_RUN_WORKSPACE_BUILD) {
        return "run.workspace.build";
    }

    if (capability == HNM_AI_CAP_USE_PROVIDER_NETWORK) {
        return "network.provider";
    }

    if (capability == HNM_AI_CAP_MODIFY_SYSTEM) {
        return "change.system";
    }

    if (capability == HNM_AI_CAP_WRITE_KERNEL) {
        return "write.kernel";
    }

    if (capability == HNM_AI_CAP_WRITE_BOOT) {
        return "write.boot";
    }

    if (capability == HNM_AI_CAP_CHANGE_POLICY) {
        return "change.policy";
    }

    if (capability == HNM_AI_CAP_CONTROL_DRIVER) {
        return "control.driver";
    }

    if (capability == HNM_AI_CAP_CHANGE_AUDIT_LOG) {
        return "change.audit";
    }

    return "unknown";
}

const char *hnm_ai_policy_decision_name(enum hnm_ai_policy_decision decision)
{
    if (decision == HNM_AI_POLICY_ALLOW) {
        return "allow";
    }

    if (decision == HNM_AI_POLICY_REQUIRE_APPROVAL) {
        return "approval";
    }

    return "deny";
}

#include <assert.h>
#include <string.h>

#include "../../kernel01/kernel/ai/ai_session.h"
#include "../../kernel01/kernel/ai/ai_workspace_policy.h"
#include "../../kernel01/kernel/ai/hnlang_ai_profile.h"
#include "../../kernel01/kernel/fs/vfs.h"

int hnm_ai_serial_transport_send_api_key(const char *api_key)
{
    return api_key != 0 && api_key[0] != '\0';
}

void hnm_log_write_line(const char *text)
{
    (void)text;
}

static void test_workspace_paths(void)
{
    assert(hnm_ai_workspace_policy_decide_path(
        HNM_AI_CAP_WRITE_HNLANG_DRAFT,
        "/workspace/main.hn") == HNM_AI_POLICY_ALLOW);
    assert(hnm_ai_workspace_policy_decide_path(
        HNM_AI_CAP_WRITE_HNLANG_DRAFT,
        "/workspace/main.c") == HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide_path(
        HNM_AI_CAP_WRITE_HNLANG_DRAFT,
        "/workspace/../kernel01/main.hn") == HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide_path(
        HNM_AI_CAP_WRITE_HNLANG_DRAFT,
        "/kernel01/main.hn") == HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide_path(
        HNM_AI_CAP_READ_BUILD_LOG,
        "/workspace/.hnmos/build.log") == HNM_AI_POLICY_ALLOW);
}

static void test_privilege_boundary(void)
{
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_RUN_WORKSPACE_BUILD) ==
        HNM_AI_POLICY_REQUIRE_APPROVAL);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_USE_PROVIDER_NETWORK) ==
        HNM_AI_POLICY_REQUIRE_APPROVAL);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_MODIFY_SYSTEM) ==
        HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_WRITE_KERNEL) ==
        HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_WRITE_BOOT) ==
        HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_CHANGE_POLICY) ==
        HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_CONTROL_DRIVER) ==
        HNM_AI_POLICY_DENY);
    assert(hnm_ai_workspace_policy_decide(HNM_AI_CAP_CHANGE_AUDIT_LOG) ==
        HNM_AI_POLICY_DENY);
}

static void test_volatile_secret_lifecycle(void)
{
    hnm_ai_session_init();
    hnm_ai_session_begin_api_key();
    assert(hnm_ai_session_capture_character('s'));
    assert(hnm_ai_session_capture_character('e'));
    assert(hnm_ai_session_capture_character('c'));
    hnm_ai_session_submit_api_key();
    assert(hnm_ai_session_secret_length() == 0);
    assert(hnm_ai_session_status() == HNM_AI_SESSION_INVALID_KEY);

    hnm_ai_session_begin_api_key();
    assert(hnm_ai_session_capture_character('a'));
    assert(hnm_ai_session_capture_character('b'));
    assert(hnm_ai_session_capture_character('c'));
    assert(hnm_ai_session_capture_character('d'));
    assert(hnm_ai_session_capture_character('e'));
    assert(hnm_ai_session_capture_character('f'));
    assert(hnm_ai_session_capture_character('g'));
    assert(hnm_ai_session_capture_character('h'));
    hnm_ai_session_submit_api_key();
    assert(hnm_ai_session_secret_length() == 0);
    assert(hnm_ai_session_status() == HNM_AI_SESSION_WAITING_PROVIDER);

    hnm_ai_session_begin_api_key();
    assert(hnm_ai_session_capture_character('x'));
    hnm_ai_session_cancel();
    assert(hnm_ai_session_secret_length() == 0);
    assert(hnm_ai_session_auth_method() == HNM_AI_AUTH_NONE);
}

static void test_workspace_ram_draft(void)
{
    const struct hnm_fs_node *node = 0;

    hnm_fs_init();
    assert(hnm_fs_node_count() == 7);
    assert(hnm_fs_write_hnlang_draft(
        "/workspace/main.hn",
        "print \"provider draft\"\n"));
    assert(hnm_fs_read("/workspace/main.hn", &node));
    assert(node != 0);
    assert(strcmp(node->data, "print \"provider draft\"\n") == 0);
    assert(!hnm_fs_write_hnlang_draft("/system/info", "blocked"));
    assert(!hnm_fs_write_hnlang_draft("/workspace/main.c", "blocked"));
}

static void test_hnlang_provider_profile(void)
{
    const char *profile = hnm_hnlang_ai_system_contract();

    assert(profile != 0);
    assert(strlen(profile) <= 512);
    assert(strstr(profile, "HNLang v0") != 0);
    assert(strstr(profile, "never modify kernel") != 0);
    assert(hnm_hnlang_ai_primitive_count() == 7);
    assert(hnm_hnlang_ai_recipe_count() == 4);
}

int main(void)
{
    test_workspace_paths();
    test_privilege_boundary();
    test_volatile_secret_lifecycle();
    test_workspace_ram_draft();
    test_hnlang_provider_profile();
    return 0;
}

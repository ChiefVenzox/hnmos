#include "ai_session.h"
#include "ai_serial_transport.h"

enum {
    HNM_AI_SESSION_SECRET_CAPACITY = 192,
    HNM_AI_SESSION_MINIMUM_KEY_LENGTH = 8,
    HNM_AI_SESSION_VISIBLE_MASK_LENGTH = 24
};

static char hnm_ai_session_secret[HNM_AI_SESSION_SECRET_CAPACITY];
static u32 hnm_ai_session_secret_index;
static enum hnm_ai_auth_method hnm_ai_session_method;
static enum hnm_ai_session_status hnm_ai_session_state;

static void hnm_ai_session_wipe_secret(void)
{
    volatile char *secret = hnm_ai_session_secret;

    for (u32 i = 0; i < sizeof(hnm_ai_session_secret); i++) {
        secret[i] = '\0';
    }

    hnm_ai_session_secret_index = 0;
}

void hnm_ai_session_init(void)
{
    hnm_ai_session_method = HNM_AI_AUTH_NONE;
    hnm_ai_session_state = HNM_AI_SESSION_DISCONNECTED;
    hnm_ai_session_wipe_secret();
}

void hnm_ai_session_begin_subscription_login(void)
{
    hnm_ai_session_wipe_secret();
    hnm_ai_session_method = HNM_AI_AUTH_SUBSCRIPTION_OAUTH;
    hnm_ai_session_state = HNM_AI_SESSION_SUBSCRIPTION_UNSUPPORTED;
}

void hnm_ai_session_begin_api_key(void)
{
    hnm_ai_session_wipe_secret();
    hnm_ai_session_method = HNM_AI_AUTH_API_KEY;
    hnm_ai_session_state = HNM_AI_SESSION_ENTERING_API_KEY;
}

int hnm_ai_session_capture_character(char character)
{
    if (hnm_ai_session_state != HNM_AI_SESSION_ENTERING_API_KEY ||
        character < 33 || character > 126 ||
        hnm_ai_session_secret_index + 1 >= sizeof(hnm_ai_session_secret)) {
        return 0;
    }

    hnm_ai_session_secret[hnm_ai_session_secret_index] = character;
    hnm_ai_session_secret_index++;
    hnm_ai_session_secret[hnm_ai_session_secret_index] = '\0';
    return 1;
}

void hnm_ai_session_backspace(void)
{
    if (hnm_ai_session_state != HNM_AI_SESSION_ENTERING_API_KEY ||
        hnm_ai_session_secret_index == 0) {
        return;
    }

    hnm_ai_session_secret_index--;
    hnm_ai_session_secret[hnm_ai_session_secret_index] = '\0';
}

void hnm_ai_session_submit_api_key(void)
{
    if (hnm_ai_session_state != HNM_AI_SESSION_ENTERING_API_KEY) {
        return;
    }

    if (hnm_ai_session_secret_index < HNM_AI_SESSION_MINIMUM_KEY_LENGTH) {
        hnm_ai_session_wipe_secret();
        hnm_ai_session_state = HNM_AI_SESSION_INVALID_KEY;
        return;
    }

    if (!hnm_ai_serial_transport_send_api_key(hnm_ai_session_secret)) {
        hnm_ai_session_wipe_secret();
        hnm_ai_session_state = HNM_AI_SESSION_INVALID_KEY;
        return;
    }

    hnm_ai_session_wipe_secret();
    hnm_ai_session_state = HNM_AI_SESSION_WAITING_PROVIDER;
}

void hnm_ai_session_cancel(void)
{
    hnm_ai_session_wipe_secret();
    hnm_ai_session_method = HNM_AI_AUTH_NONE;
    hnm_ai_session_state = HNM_AI_SESSION_DISCONNECTED;
}

int hnm_ai_session_is_capturing_secret(void)
{
    return hnm_ai_session_state == HNM_AI_SESSION_ENTERING_API_KEY;
}

u32 hnm_ai_session_secret_length(void)
{
    return hnm_ai_session_secret_index;
}

enum hnm_ai_auth_method hnm_ai_session_auth_method(void)
{
    return hnm_ai_session_method;
}

enum hnm_ai_session_status hnm_ai_session_status(void)
{
    return hnm_ai_session_state;
}

const char *hnm_ai_session_auth_method_name(void)
{
    if (hnm_ai_session_method == HNM_AI_AUTH_SUBSCRIPTION_OAUTH) {
        return "subscription OAuth";
    }

    if (hnm_ai_session_method == HNM_AI_AUTH_API_KEY) {
        return "API key";
    }

    return "none";
}

const char *hnm_ai_session_status_text(void)
{
    if (hnm_ai_session_state == HNM_AI_SESSION_ENTERING_API_KEY) {
        return "type key, ENTER submits; secret is volatile";
    }

    if (hnm_ai_session_state == HNM_AI_SESSION_WAITING_DEVICE_LOGIN) {
        return "device login staged; network runtime required";
    }

    if (hnm_ai_session_state == HNM_AI_SESSION_SUBSCRIPTION_UNSUPPORTED) {
        return "ChatGPT subscription does not authorize API usage";
    }

    if (hnm_ai_session_state == HNM_AI_SESSION_WAITING_PROVIDER) {
        return "credential handed off; local buffer cleared";
    }

    if (hnm_ai_session_state == HNM_AI_SESSION_INVALID_KEY) {
        return "key rejected and cleared";
    }

    return "offline; local stub available";
}

void hnm_ai_session_masked_secret(char *text, u32 capacity)
{
    u32 visible_count = hnm_ai_session_secret_index;
    u32 index = 0;

    if (capacity == 0) {
        return;
    }

    if (visible_count > HNM_AI_SESSION_VISIBLE_MASK_LENGTH) {
        visible_count = HNM_AI_SESSION_VISIBLE_MASK_LENGTH;
    }

    while (index < visible_count && index + 1 < capacity) {
        text[index] = '*';
        index++;
    }

    text[index] = '\0';
}

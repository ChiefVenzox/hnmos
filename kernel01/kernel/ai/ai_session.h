#ifndef HNM_AI_SESSION_H
#define HNM_AI_SESSION_H

#include "../types.h"

enum hnm_ai_auth_method {
    HNM_AI_AUTH_NONE = 0,
    HNM_AI_AUTH_SUBSCRIPTION_OAUTH,
    HNM_AI_AUTH_API_KEY
};

enum hnm_ai_session_status {
    HNM_AI_SESSION_DISCONNECTED = 0,
    HNM_AI_SESSION_ENTERING_API_KEY,
    HNM_AI_SESSION_WAITING_DEVICE_LOGIN,
    HNM_AI_SESSION_SUBSCRIPTION_UNSUPPORTED,
    HNM_AI_SESSION_WAITING_PROVIDER,
    HNM_AI_SESSION_INVALID_KEY
};

void hnm_ai_session_init(void);
void hnm_ai_session_begin_subscription_login(void);
void hnm_ai_session_begin_api_key(void);
int hnm_ai_session_capture_character(char character);
void hnm_ai_session_backspace(void);
void hnm_ai_session_submit_api_key(void);
void hnm_ai_session_cancel(void);
int hnm_ai_session_is_capturing_secret(void);
u32 hnm_ai_session_secret_length(void);
enum hnm_ai_auth_method hnm_ai_session_auth_method(void);
enum hnm_ai_session_status hnm_ai_session_status(void);
const char *hnm_ai_session_auth_method_name(void);
const char *hnm_ai_session_status_text(void);
void hnm_ai_session_masked_secret(char *text, u32 capacity);

#endif

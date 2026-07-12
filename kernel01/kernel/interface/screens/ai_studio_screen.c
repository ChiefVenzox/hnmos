#include "ai_studio_screen.h"
#include "../theme.h"
#include "../widgets/button.h"
#include "../widgets/label.h"
#include "../widgets/panel.h"
#include "../widgets/window.h"
#include "../../ai/ai_bridge.h"
#include "../../ai/ai_context.h"
#include "../../ai/ai_session.h"
#include "../../ai/ai_workspace_policy.h"
#include "../../ai/hnlang_ai_profile.h"
#include "../../fs/vfs.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/primitive.h"
#include "../../log.h"

enum {
    HNM_AI_STUDIO_BUTTON_SUBSCRIPTION = 0,
    HNM_AI_STUDIO_BUTTON_API_KEY,
    HNM_AI_STUDIO_BUTTON_GENERATE,
    HNM_AI_STUDIO_BUTTON_REVIEW,
    HNM_AI_STUDIO_BUTTON_BUILD,
    HNM_AI_STUDIO_BUTTON_COUNT
};

static const struct hnm_ui_theme *hnm_ai_studio_theme;
static struct hnm_ui_button hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_COUNT];
static int hnm_ai_studio_pressed_index;
static int hnm_ai_studio_ready;
static const char *hnm_ai_studio_activity;
static char hnm_ai_studio_prompt[HNM_AI_TASK_INPUT_MAX];
static u32 hnm_ai_studio_prompt_length;
static u32 hnm_ai_studio_saved_task_id;
static int hnm_ai_studio_capturing_prompt;

static void hnm_ai_studio_label(
    u32 x,
    u32 y,
    const char *text,
    hnm_color foreground,
    hnm_color background)
{
    struct hnm_ui_label label;

    label.x = x;
    label.y = y;
    label.text = text;
    label.foreground = foreground;
    label.background = background;
    hnm_ui_label_render(&label);
}

static void hnm_ai_studio_set_button(
    u32 index,
    u32 x,
    u32 y,
    u32 width,
    u32 height,
    const char *text)
{
    struct hnm_ui_button *button = &hnm_ai_studio_buttons[index];

    button->rect.x = x;
    button->rect.y = y;
    button->rect.width = width;
    button->rect.height = height;
    button->text = text;
    button->normal_background = hnm_ai_studio_theme->button;
    button->hover_background = hnm_ai_studio_theme->button_hover;
    button->pressed_background = hnm_ai_studio_theme->button_pressed;
    button->border = hnm_ai_studio_theme->border;
    button->text_color = hnm_ai_studio_theme->button_text;
}

static void hnm_ai_studio_render_connection(u32 x, u32 y, u32 width, u32 height)
{
    struct hnm_ui_panel panel;
    char masked_secret[32];
    const char *status_line = "COM2 provider bridge ready";
    const char *status_detail = "enter key or use host env";

    if (hnm_ai_session_status() == HNM_AI_SESSION_ENTERING_API_KEY) {
        status_line = "type key; ENTER submits";
        status_detail = "masked + volatile";
    } else if (hnm_ai_session_status() == HNM_AI_SESSION_WAITING_DEVICE_LOGIN) {
        status_line = "device login staged";
        status_detail = "network runtime required";
    } else if (hnm_ai_session_status() == HNM_AI_SESSION_SUBSCRIPTION_UNSUPPORTED) {
        status_line = "subscription is separate";
        status_detail = "use an API platform key";
    } else if (hnm_ai_session_status() == HNM_AI_SESSION_WAITING_PROVIDER) {
        status_line = "credential handed off";
        status_detail = "local secret cleared";
    } else if (hnm_ai_session_status() == HNM_AI_SESSION_INVALID_KEY) {
        status_line = "key rejected + cleared";
        status_detail = "minimum length is 8";
    }

    panel.rect.x = x;
    panel.rect.y = y;
    panel.rect.width = width;
    panel.rect.height = height;
    panel.background = hnm_ai_studio_theme->panel;
    panel.border = hnm_ai_studio_theme->border;
    hnm_ui_panel_render(&panel);

    hnm_ai_studio_label(x + 16, y + 16, "AI CONNECTION", hnm_ai_studio_theme->text, panel.background);
    hnm_ai_studio_label(x + 16, y + 38, "provider-neutral session", hnm_ai_studio_theme->muted_text, panel.background);

    hnm_ai_studio_set_button(
        HNM_AI_STUDIO_BUTTON_SUBSCRIPTION,
        x + 16,
        y + 68,
        width - 32,
        36,
        "L  Subscription info");
    hnm_ai_studio_set_button(
        HNM_AI_STUDIO_BUTTON_API_KEY,
        x + 16,
        y + 116,
        width - 32,
        36,
        "K  Add API key");
    hnm_ui_button_render(&hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_SUBSCRIPTION]);
    hnm_ui_button_render(&hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_API_KEY]);

    hnm_ai_studio_label(x + 16, y + 168, "auth:", hnm_ai_studio_theme->muted_text, panel.background);
    hnm_ai_studio_label(x + 64, y + 168, hnm_ai_session_auth_method_name(), hnm_ai_studio_theme->text, panel.background);
    hnm_ai_studio_label(x + 16, y + 190, status_line, hnm_ai_studio_theme->accent_alt, panel.background);
    hnm_ai_studio_label(x + 16, y + 210, status_detail, hnm_ai_studio_theme->muted_text, panel.background);

    hnm_draw_fill_rect(x + 16, y + 232, width - 32, 32, hnm_ai_studio_theme->background);
    hnm_draw_rect(x + 16, y + 232, width - 32, 32, hnm_ai_studio_theme->accent);
    hnm_ai_session_masked_secret(masked_secret, sizeof(masked_secret));
    hnm_ai_studio_label(
        x + 24,
        y + 244,
        masked_secret[0] != '\0' ? masked_secret : "secret is never displayed",
        hnm_ai_studio_theme->muted_text,
        hnm_ai_studio_theme->background);

    hnm_ai_studio_label(x + 16, y + 286, "WORKSPACE POLICY", hnm_ai_studio_theme->text, panel.background);
    hnm_ai_studio_label(x + 16, y + 312, "+ write HNLang drafts", hnm_ai_studio_theme->accent_alt, panel.background);
    hnm_ai_studio_label(x + 16, y + 334, "+ inspect build logs", hnm_ai_studio_theme->accent_alt, panel.background);
    hnm_ai_studio_label(x + 16, y + 356, "? build/network approval", hnm_ai_studio_theme->accent, panel.background);
    hnm_ai_studio_label(x + 16, y + 378, "- kernel / boot writes", hnm_ai_studio_theme->muted_text, panel.background);
    hnm_ai_studio_label(x + 16, y + 400, "- policy / driver control", hnm_ai_studio_theme->muted_text, panel.background);
    hnm_ai_studio_label(x + 16, y + 422, "deny by default", hnm_ai_studio_theme->accent_alt, panel.background);

    hnm_ai_studio_label(x + 16, y + height - 48, "Credentials do not grant", hnm_ai_studio_theme->muted_text, panel.background);
    hnm_ai_studio_label(x + 16, y + height - 26, "HNMos system authority.", hnm_ai_studio_theme->text, panel.background);
}

static void hnm_ai_studio_sync_provider_result(void)
{
    const struct hnm_ai_task *task = hnm_ai_bridge_last_task();

    if (task == 0 || task->task_type != HNM_AI_TASK_HNLANG || task->task_id == 0) {
        return;
    }

    if (task->status == AI_TASK_RUNNING || task->status == AI_TASK_PENDING) {
        hnm_ai_studio_activity = "Provider request running over COM2 bridge";
        return;
    }

    if (task->status == AI_TASK_ERROR) {
        hnm_ai_studio_activity = "Provider request failed; verify bridge/key/model";
        return;
    }

    if (task->status == AI_TASK_CANCELLED) {
        hnm_ai_studio_activity = "Provider request cancelled locally";
        return;
    }

    if (task->status == AI_TASK_DONE && task->task_id != hnm_ai_studio_saved_task_id) {
        if (hnm_fs_write_hnlang_draft("/workspace/main.hn", task->output)) {
            hnm_ai_studio_saved_task_id = task->task_id;
            hnm_ai_studio_activity = "Provider draft saved to /workspace/main.hn";
        } else {
            hnm_ai_studio_activity = "Provider result blocked by workspace policy";
        }
    }
}

static void hnm_ai_studio_render_source(u32 x, u32 y, hnm_color background)
{
    const struct hnm_fs_node *node = 0;
    const char *source = "# /workspace/main.hn unavailable";
    u32 source_index = 0;

    if (hnm_fs_read("/workspace/main.hn", &node) && node != 0 && node->data != 0) {
        source = node->data;
    }

    for (u32 visual_line = 0; visual_line < 9 && source[source_index] != '\0'; visual_line++) {
        char line[68];
        char line_number[3];
        u32 line_length = 0;
        u32 line_y = y + (visual_line * 24);

        while (source[source_index] != '\0' &&
            source[source_index] != '\n' &&
            line_length + 1 < sizeof(line)) {
            char character = source[source_index];

            line[line_length] = character == '\t' ? ' ' : character;
            line_length++;
            source_index++;
        }

        line[line_length] = '\0';
        line_number[0] = (char)('0' + ((visual_line + 1) / 10));
        line_number[1] = (char)('0' + ((visual_line + 1) % 10));
        line_number[2] = '\0';
        hnm_ai_studio_label(x, line_y, line_number, hnm_ai_studio_theme->muted_text, hnm_ai_studio_theme->panel_alt);
        hnm_ai_studio_label(x + 40, line_y, line, hnm_ai_studio_theme->text, background);

        while (source[source_index] != '\0' && source[source_index] != '\n') {
            source_index++;
        }

        if (source[source_index] == '\n') {
            source_index++;
        }
    }
}

static void hnm_ai_studio_render_code(u32 x, u32 y, u32 width, u32 height)
{
    struct hnm_ui_window window;
    struct hnm_ui_panel assistant;
    char prompt_preview[64];
    u32 body_y;
    u32 editor_height = 286;
    u32 assistant_y;
    u32 action_y;

    for (u32 i = 0; i < sizeof(prompt_preview); i++) {
        prompt_preview[i] = '\0';
    }

    for (u32 i = 0;
        hnm_ai_studio_prompt[i] != '\0' && i + 1 < sizeof(prompt_preview);
        i++) {
        prompt_preview[i] = hnm_ai_studio_prompt[i];
    }

    window.rect.x = x;
    window.rect.y = y;
    window.rect.width = width;
    window.rect.height = height;
    window.title = "HN AI Studio  |  main.hn  |  HNLang AI profile";
    window.title_background = hnm_ai_studio_theme->accent;
    window.title_text = hnm_ai_studio_theme->text;
    window.body_background = hnm_ai_studio_theme->panel;
    window.border = hnm_ai_studio_theme->border;
    hnm_ui_window_render(&window);

    body_y = y + hnm_ui_window_title_height();
    hnm_draw_fill_rect(x + 12, body_y + 12, width - 24, editor_height, hnm_ai_studio_theme->background);
    hnm_draw_rect(x + 12, body_y + 12, width - 24, editor_height, hnm_ai_studio_theme->border);
    hnm_draw_fill_rect(x + 13, body_y + 13, 38, editor_height - 2, hnm_ai_studio_theme->panel_alt);

    hnm_ai_studio_render_source(x + 22, body_y + 30, hnm_ai_studio_theme->background);

    hnm_ai_studio_label(x + 62, body_y + 196, hnm_hnlang_ai_profile_summary(), hnm_ai_studio_theme->muted_text, hnm_ai_studio_theme->background);
    hnm_ai_studio_label(x + 62, body_y + 220, "Combinations are bounded, typed, reusable and reviewable.", hnm_ai_studio_theme->muted_text, hnm_ai_studio_theme->background);

    action_y = body_y + editor_height + 24;
    hnm_ai_studio_set_button(HNM_AI_STUDIO_BUTTON_GENERATE, x + 12, action_y, 152, 36, "G  Provider prompt");
    hnm_ai_studio_set_button(HNM_AI_STUDIO_BUTTON_REVIEW, x + 176, action_y, 136, 36, "R  Review");
    hnm_ai_studio_set_button(HNM_AI_STUDIO_BUTTON_BUILD, x + 324, action_y, 152, 36, "B  Build request");
    hnm_ui_button_render(&hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_GENERATE]);
    hnm_ui_button_render(&hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_REVIEW]);
    hnm_ui_button_render(&hnm_ai_studio_buttons[HNM_AI_STUDIO_BUTTON_BUILD]);

    assistant_y = action_y + 52;
    assistant.rect.x = x + 12;
    assistant.rect.y = assistant_y;
    assistant.rect.width = width - 24;
    assistant.rect.height = height > assistant_y - y + 12 ? height - (assistant_y - y) - 12 : 0;
    assistant.background = hnm_ai_studio_theme->panel_alt;
    assistant.border = hnm_ai_studio_theme->accent_alt;
    hnm_ui_panel_render(&assistant);
    hnm_ai_studio_label(x + 26, assistant_y + 14, "ASSISTANT / TASK PREVIEW", hnm_ai_studio_theme->text, assistant.background);
    hnm_ai_studio_label(x + 26, assistant_y + 38, "prompt:", hnm_ai_studio_theme->muted_text, assistant.background);
    hnm_ai_studio_label(
        x + 90,
        assistant_y + 38,
        prompt_preview[0] != '\0' ? prompt_preview : "G starts a provider prompt",
        hnm_ai_studio_theme->text,
        assistant.background);
    hnm_ai_studio_label(x + 26, assistant_y + 62, hnm_ai_studio_activity, hnm_ai_studio_theme->accent_alt, assistant.background);
    hnm_ai_studio_label(x + 26, assistant_y + 86, "AI drafts; policy gates; /workspace only. X cancels.", hnm_ai_studio_theme->muted_text, assistant.background);
}

void hnm_ai_studio_screen_init(void)
{
    hnm_ai_studio_theme = hnm_ui_theme_default();
    hnm_ai_studio_pressed_index = -1;
    hnm_ai_studio_activity = "Ready: OpenAI COM2 bridge + HNLang profile";
    hnm_ai_studio_prompt[0] = '\0';
    hnm_ai_studio_prompt_length = 0;
    hnm_ai_studio_saved_task_id = 0;
    hnm_ai_studio_capturing_prompt = 0;
    hnm_ai_session_init();
    hnm_ai_studio_ready = hnm_framebuffer_is_available();
}

void hnm_ai_studio_screen_render(void)
{
    const struct hnm_framebuffer *fb = hnm_framebuffer_get();
    u32 margin = 24;
    u32 gap = 16;
    u32 content_y = 108;
    u32 content_height;
    u32 left_width = 264;
    u32 right_x;
    u32 right_width;

    if (!hnm_ai_studio_ready || fb->width <= (margin * 2) || fb->height <= content_y + margin) {
        return;
    }

    hnm_ai_studio_sync_provider_result();

    content_height = fb->height - content_y - margin;
    right_x = margin + left_width + gap;
    right_width = fb->width > right_x + margin ? fb->width - right_x - margin : 0;

    hnm_draw_fill_rect(0, 84, fb->width, fb->height - 84, hnm_ai_studio_theme->background);
    hnm_ai_studio_render_connection(margin, content_y, left_width, content_height);

    if (right_width > 480) {
        hnm_ai_studio_render_code(right_x, content_y, right_width, content_height);
    }
}

static void hnm_ai_studio_generate(void)
{
    const struct hnm_ai_task *task = hnm_ai_bridge_last_task();

    if (task != 0 && task->status == AI_TASK_RUNNING) {
        hnm_ai_studio_activity = "A provider request is running; X cancels locally";
        return;
    }

    if (hnm_ai_session_is_capturing_secret()) {
        hnm_ai_session_cancel();
    }

    hnm_ai_studio_prompt[0] = '\0';
    hnm_ai_studio_prompt_length = 0;
    hnm_ai_studio_capturing_prompt = 1;
    hnm_ai_studio_activity = "Type an HNLang app request, then press ENTER";
}

static void hnm_ai_studio_submit_prompt(void)
{
    if (hnm_ai_studio_prompt_length == 0) {
        hnm_ai_studio_activity = "Prompt is empty; type a request first";
        return;
    }

    if (hnm_ai_workspace_policy_decide_path(
            HNM_AI_CAP_WRITE_HNLANG_DRAFT,
            "/workspace/main.hn") != HNM_AI_POLICY_ALLOW) {
        hnm_ai_studio_activity = "Denied: workspace draft capability unavailable";
        return;
    }

    if (!hnm_ai_bridge_submit(
            HNM_AI_TASK_HNLANG,
            hnm_ai_studio_prompt,
            HNM_AI_CONTEXT_HNLANG_TASK)) {
        hnm_ai_studio_activity = "Draft failed: AI bridge unavailable";
        return;
    }

    hnm_ai_studio_capturing_prompt = 0;
    hnm_ai_studio_activity = "Request sent; waiting for OpenAI bridge response";
}

static void hnm_ai_studio_review(void)
{
    hnm_ai_studio_activity = "Review passed: workspace-only HNLang proposal";
}

static void hnm_ai_studio_build(void)
{
    enum hnm_ai_policy_decision decision =
        hnm_ai_workspace_policy_decide(HNM_AI_CAP_RUN_WORKSPACE_BUILD);

    if (decision == HNM_AI_POLICY_REQUIRE_APPROVAL) {
        hnm_ai_studio_activity = "Build blocked: explicit user approval is required";
    } else if (decision == HNM_AI_POLICY_ALLOW) {
        hnm_ai_studio_activity = "Build request queued inside workspace";
    } else {
        hnm_ai_studio_activity = "Build denied by workspace policy";
    }
}

static void hnm_ai_studio_activate(u32 index)
{
    if (index == HNM_AI_STUDIO_BUTTON_SUBSCRIPTION) {
        hnm_ai_studio_capturing_prompt = 0;
        hnm_ai_session_begin_subscription_login();
        hnm_ai_studio_activity = "ChatGPT subscription cannot authorize API usage";
    } else if (index == HNM_AI_STUDIO_BUTTON_API_KEY) {
        hnm_ai_studio_capturing_prompt = 0;
        hnm_ai_session_begin_api_key();
        hnm_ai_studio_activity = "Type API key; ENTER hands it to the bridge";
    } else if (index == HNM_AI_STUDIO_BUTTON_GENERATE) {
        hnm_ai_studio_generate();
    } else if (index == HNM_AI_STUDIO_BUTTON_REVIEW) {
        hnm_ai_studio_review();
    } else if (index == HNM_AI_STUDIO_BUTTON_BUILD) {
        hnm_ai_studio_build();
    }

    hnm_ai_studio_screen_render();
}

static void hnm_ai_studio_update_hover(u32 x, u32 y)
{
    int changed = 0;

    for (u32 i = 0; i < HNM_AI_STUDIO_BUTTON_COUNT; i++) {
        int hovered = hnm_ui_button_hit_test(&hnm_ai_studio_buttons[i], x, y);

        if (hnm_ai_studio_buttons[i].hovered != hovered) {
            hnm_ai_studio_buttons[i].hovered = hovered;
            changed = 1;
        }
    }

    if (changed) {
        hnm_ai_studio_screen_render();
    }
}

static int hnm_ai_studio_mouse_down(u32 x, u32 y)
{
    hnm_ai_studio_pressed_index = -1;
    hnm_ai_studio_update_hover(x, y);

    for (u32 i = 0; i < HNM_AI_STUDIO_BUTTON_COUNT; i++) {
        int pressed = hnm_ai_studio_buttons[i].hovered;

        hnm_ai_studio_buttons[i].pressed = pressed;

        if (pressed) {
            hnm_ai_studio_pressed_index = (int)i;
        }
    }

    hnm_ai_studio_screen_render();
    return hnm_ai_studio_pressed_index >= 0;
}

static int hnm_ai_studio_mouse_up(u32 x, u32 y)
{
    int pressed_index = hnm_ai_studio_pressed_index;

    hnm_ai_studio_pressed_index = -1;

    for (u32 i = 0; i < HNM_AI_STUDIO_BUTTON_COUNT; i++) {
        hnm_ai_studio_buttons[i].pressed = 0;
    }

    if (pressed_index >= 0 &&
        hnm_ui_button_hit_test(&hnm_ai_studio_buttons[pressed_index], x, y)) {
        hnm_ai_studio_activate((u32)pressed_index);
        return 1;
    }

    hnm_ai_studio_screen_render();
    return 0;
}

int hnm_ai_studio_screen_handle_event(const struct hnm_ui_event *event)
{
    if (!hnm_ai_studio_ready) {
        return 0;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_BACKSPACE &&
        hnm_ai_session_is_capturing_secret()) {
        hnm_ai_session_backspace();
        hnm_ai_studio_screen_render();
        return 1;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_BACKSPACE &&
        hnm_ai_studio_capturing_prompt) {
        if (hnm_ai_studio_prompt_length > 0) {
            hnm_ai_studio_prompt_length--;
            hnm_ai_studio_prompt[hnm_ai_studio_prompt_length] = '\0';
        }
        hnm_ai_studio_screen_render();
        return 1;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_ENTER &&
        hnm_ai_session_is_capturing_secret()) {
        hnm_ai_session_submit_api_key();
        hnm_ai_studio_activity = "API key handed to bridge; HNMos copy cleared";
        hnm_ai_studio_screen_render();
        return 1;
    }

    if (event->type == HNM_UI_EVENT_KEY_DOWN &&
        event->key_code == HNM_UI_KEY_ENTER &&
        hnm_ai_studio_capturing_prompt) {
        hnm_ai_studio_submit_prompt();
        hnm_ai_studio_screen_render();
        return 1;
    }

    if (event->type == HNM_UI_EVENT_KEY_CHAR) {
        if (hnm_ai_session_is_capturing_secret()) {
            hnm_ai_session_capture_character(event->character);
            hnm_ai_studio_screen_render();
            return 1;
        }

        if (hnm_ai_studio_capturing_prompt) {
            if (event->character >= 32 && event->character <= 126 &&
                hnm_ai_studio_prompt_length + 1 < sizeof(hnm_ai_studio_prompt)) {
                hnm_ai_studio_prompt[hnm_ai_studio_prompt_length] = event->character;
                hnm_ai_studio_prompt_length++;
                hnm_ai_studio_prompt[hnm_ai_studio_prompt_length] = '\0';
            }
            hnm_ai_studio_screen_render();
            return 1;
        }

        if (event->character == 'l') {
            hnm_ai_studio_activate(HNM_AI_STUDIO_BUTTON_SUBSCRIPTION);
            return 1;
        }

        if (event->character == 'k') {
            hnm_ai_studio_activate(HNM_AI_STUDIO_BUTTON_API_KEY);
            return 1;
        }

        if (event->character == 'g') {
            hnm_ai_studio_activate(HNM_AI_STUDIO_BUTTON_GENERATE);
            return 1;
        }

        if (event->character == 'r') {
            hnm_ai_studio_activate(HNM_AI_STUDIO_BUTTON_REVIEW);
            return 1;
        }

        if (event->character == 'b') {
            hnm_ai_studio_activate(HNM_AI_STUDIO_BUTTON_BUILD);
            return 1;
        }

        if (event->character == 'x') {
            if (hnm_ai_bridge_cancel()) {
                hnm_ai_studio_activity = "Provider request cancelled locally";
            } else {
                hnm_ai_studio_activity = "No running provider request";
            }
            hnm_ai_studio_screen_render();
            return 1;
        }
    }

    if (event->type == HNM_UI_EVENT_MOUSE_MOVE) {
        hnm_ai_studio_update_hover(event->x, event->y);
        return 0;
    }

    if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_DOWN &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        return hnm_ai_studio_mouse_down(event->x, event->y);
    }

    if (event->type == HNM_UI_EVENT_MOUSE_BUTTON_UP &&
        event->button == HNM_UI_MOUSE_BUTTON_LEFT) {
        return hnm_ai_studio_mouse_up(event->x, event->y);
    }

    return 0;
}

void hnm_ai_studio_screen_on_enter(void)
{
    hnm_ai_studio_pressed_index = -1;
    hnm_log_write_line("router: AI Studio entered.");
}

void hnm_ai_studio_screen_on_exit(void)
{
    if (hnm_ai_session_is_capturing_secret()) {
        hnm_ai_session_cancel();
    }

    hnm_ai_studio_pressed_index = -1;
    hnm_ai_studio_capturing_prompt = 0;
    hnm_log_write_line("router: AI Studio exited; volatile secret cleared.");
}

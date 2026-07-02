#include "window.h"
#include "../../graphics/font.h"
#include "../../graphics/primitive.h"

enum {
    HNM_UI_WINDOW_TITLE_HEIGHT = 24
};

u32 hnm_ui_window_title_height(void)
{
    return HNM_UI_WINDOW_TITLE_HEIGHT;
}

void hnm_ui_window_render(const struct hnm_ui_window *window)
{
    u32 body_y = window->rect.y + HNM_UI_WINDOW_TITLE_HEIGHT;
    u32 body_height = window->rect.height > HNM_UI_WINDOW_TITLE_HEIGHT ?
        window->rect.height - HNM_UI_WINDOW_TITLE_HEIGHT :
        0;

    hnm_draw_fill_rect(
        window->rect.x,
        window->rect.y,
        window->rect.width,
        HNM_UI_WINDOW_TITLE_HEIGHT,
        window->title_background);
    hnm_draw_fill_rect(
        window->rect.x,
        body_y,
        window->rect.width,
        body_height,
        window->body_background);
    hnm_draw_rect(
        window->rect.x,
        window->rect.y,
        window->rect.width,
        window->rect.height,
        window->border);
    hnm_font_draw_string(
        window->rect.x + 8,
        window->rect.y + 8,
        window->title,
        window->title_text,
        window->title_background);
}

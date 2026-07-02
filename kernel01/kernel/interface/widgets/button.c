#include "button.h"
#include "../../graphics/font.h"
#include "../../graphics/primitive.h"

static hnm_color hnm_ui_button_background(const struct hnm_ui_button *button)
{
    if (button->pressed) {
        return button->pressed_background;
    }

    if (button->hovered) {
        return button->hover_background;
    }

    return button->normal_background;
}

void hnm_ui_button_render(const struct hnm_ui_button *button)
{
    hnm_color background = hnm_ui_button_background(button);
    u32 text_width = hnm_font_string_width(button->text);
    u32 text_x = button->rect.x + 8;
    u32 text_y = button->rect.y + ((button->rect.height - HNM_FONT_HEIGHT) / 2);

    if (button->rect.width > text_width) {
        text_x = button->rect.x + ((button->rect.width - text_width) / 2);
    }

    hnm_draw_fill_rect(button->rect.x, button->rect.y, button->rect.width, button->rect.height, background);
    hnm_draw_rect(button->rect.x, button->rect.y, button->rect.width, button->rect.height, button->border);
    hnm_font_draw_string(text_x, text_y, button->text, button->text_color, background);
}

int hnm_ui_button_hit_test(const struct hnm_ui_button *button, u32 x, u32 y)
{
    return hnm_ui_point_in_rect(x, y, &button->rect);
}

#include "label.h"
#include "../../graphics/font.h"

void hnm_ui_label_render(const struct hnm_ui_label *label)
{
    hnm_font_draw_string(
        label->x,
        label->y,
        label->text,
        label->foreground,
        label->background);
}

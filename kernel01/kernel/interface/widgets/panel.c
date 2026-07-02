#include "panel.h"
#include "../../graphics/primitive.h"

void hnm_ui_panel_render(const struct hnm_ui_panel *panel)
{
    hnm_draw_fill_rect(
        panel->rect.x,
        panel->rect.y,
        panel->rect.width,
        panel->rect.height,
        panel->background);
    hnm_draw_rect(
        panel->rect.x,
        panel->rect.y,
        panel->rect.width,
        panel->rect.height,
        panel->border);
}

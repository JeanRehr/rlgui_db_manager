/**
 * @file Scrollpanel.c
 * @brief Scrollpanel implementation
 */
#include "ui_elements/scrollpanel.h"

#include <external/raylib/raygui.h>

struct scrollpanel scrollpanel_init(Rectangle panel_bounds, const char *title, Rectangle panel_content_bounds) {
    struct scrollpanel sp = { 0 };
    sp.panel_bounds = panel_bounds;
    sp.title = title;
    sp.panel_content_bounds = panel_content_bounds;
    return sp;
}

void scrollpanel_draw(struct scrollpanel *sp, void (*draw_content)(Rectangle, char *), char *content_text) {
    GuiScrollPanel(sp->panel_bounds, sp->title, sp->panel_content_bounds, &sp->scroll, &sp->view);

    Rectangle content_rect = (Rectangle) { sp->panel_bounds.x + sp->scroll.x,
                                           sp->panel_bounds.y + sp->scroll.y,
                                           sp->panel_content_bounds.width,
                                           sp->panel_content_bounds.height };

    BeginScissorMode(
        sp->view.x,
        sp->view.y,
        sp->view.width,
        sp->view.height
    );
    draw_content(content_rect, content_text);
    EndScissorMode();
}

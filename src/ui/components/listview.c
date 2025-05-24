/**
 * @file listview.c
 * @brief Listview implementation
 */
#include "ui/components/listview.h"

#include <external/raylib/raygui.h>

#include "global/CONSTANTS.h"

struct listview listview_init(Rectangle bounds, const char *title, const char *options) {
    struct listview lv = { 0 };
    lv.bounds = bounds;
    lv.title = title;
    lv.options = options;
    lv.scrollindex = 0;
    lv.active_option = 0;
    return lv;
}

void listview_draw(struct listview *lv) {
    GuiLabel((Rectangle) { lv->bounds.x, lv->bounds.y - (FONT_SIZE + 5), lv->bounds.width, 20 }, lv->title);

    GuiListView(lv->bounds, lv->options, &lv->scrollindex, &lv->active_option);
}

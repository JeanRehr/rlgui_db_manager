/**
 * @file dropdownbox.c
 * @brief Dropdownbox implementation
 */
#include "ui/components/dropdownbox.h"

#include <external/raylib/raygui.h>

#include "global/CONSTANTS.h"

struct dropdownbox dropdownbox_init(Rectangle bounds, const char *options, const char *title) {
    struct dropdownbox ddb = { 0 };
    ddb.bounds = bounds;
    ddb.options = options;
    ddb.title = title;
    ddb.active_option = 0;
    ddb.edit_mode = false;
    return ddb;
}

void dropdownbox_draw(struct dropdownbox *ddb) {
    GuiLabel((Rectangle) { ddb->bounds.x, ddb->bounds.y - (FONT_SIZE + 5), ddb->bounds.width, 20 }, ddb->title);

    if (GuiDropdownBox(ddb->bounds, ddb->options, &ddb->active_option, ddb->edit_mode)) {
        ddb->edit_mode = !ddb->edit_mode;
    }
}

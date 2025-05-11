/**
 * @file checkbox.c
 * @brief Checkbox implementation
 */
#include "ui_elements/checkbox.h"

#include <stdbool.h>
#include <stddef.h>

#include <external/raylib/raygui.h>

#include "CONSTANTS.h"

struct checkbox checkbox_init(Rectangle bounds, const char *title) {
    struct checkbox cb = { 0 };
    cb.bounds = bounds;
    cb.title = title;
    cb.checked = false;
    return cb;
}

int checkbox_draw(struct checkbox *cb) {
    GuiLabel(
        (Rectangle) { cb->bounds.x, cb->bounds.y - (FONT_SIZE + 5), MeasureText(cb->title, FONT_SIZE) + 10, 20 },
        cb->title
    );

    return GuiCheckBox(cb->bounds, NULL, &cb->checked);
}

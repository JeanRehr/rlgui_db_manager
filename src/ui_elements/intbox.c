/**
 * @file intbox.c
 * @brief Intbox implementation
 */
#include "ui_elements/intbox.h"

#include <external/raylib/raygui.h>

#include <stddef.h>

#include "CONSTANTS.h"

struct intbox intbox_init(Rectangle bounds, const char *title, const int min_val, const int max_val) {
    struct intbox ib = { 0 };
    ib.bounds = bounds;
    ib.title = title;
    ib.input = 0;
    ib.edit_mode = false;
    ib.min_val = min_val;
    ib.max_val = max_val;
    return ib;
}

void intbox_draw(struct intbox *ib) {
    // Check whether the provided width of the box is higher than the width of the title.
    // If not, use the width of the title as width of the GuiLabel
    int title_width =
        MeasureText(ib->title, FONT_SIZE) > ib->bounds.width ? MeasureText(ib->title, FONT_SIZE) : ib->bounds.width;
    // Draw title above the intbox
    GuiLabel((Rectangle) { ib->bounds.x, ib->bounds.y - (FONT_SIZE + 5), title_width, 20 }, ib->title);

    if (GuiValueBox(ib->bounds, NULL, &ib->input, ib->min_val, ib->max_val, ib->edit_mode)) {
        ib->edit_mode = !ib->edit_mode;
    }
}

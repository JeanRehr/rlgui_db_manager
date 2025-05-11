/**
 * @file textboxint.c
 * @brief Textboxint implementation
 */
#include "ui_elements/textboxint.h"

#include <external/raylib/raygui.h>

#include <stdbool.h>

#include "utilsfn.h"

struct textboxint textboxint_init(Rectangle bounds, const char *title) {
    struct textboxint tbi = { 0 }; // Initialize struct with zeroed memory
    // input already initialized to 0 with above
    tbi.bounds = bounds;
    tbi.edit_mode = false;
    tbi.title = title;
    return tbi;
}

void textboxint_draw(struct textboxint *tbi) {
    // Draw title above the textboxint
    GuiLabel((Rectangle) { tbi->bounds.x, tbi->bounds.y - (FONT_SIZE + 5), tbi->bounds.width, 20 }, tbi->title);

    // Draw and manage textboxint
    if (GuiTextBox(tbi->bounds, tbi->input, MAX_INPUT, tbi->edit_mode)) {
        tbi->edit_mode = !tbi->edit_mode;
    }

    // Filter input to only ints
    if (tbi->edit_mode) {
        filter_integer_input(tbi->input, MAX_INPUT);
    }
}

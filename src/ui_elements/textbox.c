/**
 * @file textbox.c
 * @brief Textbox implementation
 */
#include "ui_elements/textbox.h"

#include <external/raylib/raygui.h>

#include "CONSTANTS.h"

struct textbox textbox_init(Rectangle bounds, const char *title) {
    struct textbox textbox = { 0 }; // Initialize struct with zeroed memory
    // input already initialized to 0 with above
    textbox.bounds = bounds;
    textbox.edit_mode = false;
    textbox.title = title;
    return textbox;
}

void textbox_draw(struct textbox *textbox) {
    // Draw title above the textbox
    GuiLabel(
        (Rectangle) { textbox->bounds.x, textbox->bounds.y - (FONT_SIZE + 5), textbox->bounds.width, 20 },
        textbox->title
    );

    // Draw and manage textbox
    if (GuiTextBox(textbox->bounds, textbox->input, MAX_INPUT, textbox->edit_mode)) {
        textbox->edit_mode = !textbox->edit_mode;
    }
}

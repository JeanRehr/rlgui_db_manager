#include <external/raylib/raygui.h>
#include <string.h>

#include "CONSTANTS.h"
#include "ui_elements/textbox.h"
#include "utilsfn.h"

struct textbox textbox_init(Rectangle bounds, const char *label) {
    struct textbox textbox = { 0 }; // Initialize struct with zeroed memory
    // input already initialized to 0 with above
    textbox.bounds = bounds;
    textbox.edit_mode = false;
    textbox.label = label;
    return textbox;
}

void textbox_draw(struct textbox *textbox) {
    // Draw label above the textbox
    GuiLabel(
        (Rectangle) { textbox->bounds.x, textbox->bounds.y - (FONT_SIZE + 5), textbox->bounds.width, 20 },
        textbox->label
    );

    // Draw and manage textbox
    if (GuiTextBox(textbox->bounds, textbox->input, MAX_INPUT, textbox->edit_mode)) {
        textbox->edit_mode = !textbox->edit_mode;
    }
}

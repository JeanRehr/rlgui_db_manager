#include <external/raylib/raygui.h>

#include <stdbool.h>
#include <string.h>

#include "CONSTANTS.h"
#include "ui_elements/button.h"

// Function to initialize a Button with given properties
struct button button_init(Rectangle bounds, const char *label) {
    struct button button = { 0 };

    // Calculate the width of the label
    int label_width = MeasureText(label, FONT_SIZE);
    if (label_width > bounds.width) {
        button.bounds.x = bounds.x;
        button.bounds.y = bounds.y;
        button.bounds.width = label_width + 10; // +10 to give it some space
        button.bounds.height = bounds.height;
    } else {
        button.bounds = bounds;
    }

    button.label = label;
    return button;
}

int button_draw_updt(struct button *button) {
    return GuiButton(button->bounds, button->label);
}

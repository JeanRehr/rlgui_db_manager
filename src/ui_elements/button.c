/**
 * @file button.c
 * @brief Button element implementation
 */
#include "ui_elements/button.h"

#include <external/raylib/raygui.h>

#include "CONSTANTS.h"

struct button button_init(Rectangle bounds, const char *title) {
    struct button button = { 0 };

    // Calculate the width of the title
    int title_width = MeasureText(title, FONT_SIZE);
    if (title_width > bounds.width) {
        button.bounds.x = bounds.x;
        button.bounds.y = bounds.y;
        button.bounds.width = title_width + 10; // +10 to give it some space
        button.bounds.height = bounds.height;
    } else {
        button.bounds = bounds;
    }

    button.title = title;
    return button;
}

int button_draw_updt(struct button *button) {
    return GuiButton(button->bounds, button->title);
}

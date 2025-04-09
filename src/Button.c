#include <raygui.h>

#include <string.h>
#include <stdbool.h>

#include "Button.h"
#include "CONSTANTS.h"

typedef struct Button Button; // Declare the Button struct type

// Function to initialize a Button with given properties
Button buttonInit(Rectangle bounds, const char* label) {
    Button button = {0};
    button.bounds = bounds;
    button.label = label;
    return button;
}

int buttonDrawUpdt(Button *button) {
    return GuiButton(button->bounds, button->label);
}
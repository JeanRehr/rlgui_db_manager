#include <raylib.h>
#include <string.h>
#include <stdbool.h>

#include "Button.h"
#include "CONSTANTS.h"

typedef struct Button Button; // Declare the Button struct type

// Function to initialize a Button with given properties
void buttonInit(Button *button, float posX, float posY, float width, float height, const char* title, Color colorText, Color color, Color colorLine, Color colorActive) {
    // Set position and size of the button
    button->rect.x = posX;
    button->rect.y = posY;
    button->rect.width = width;
    button->rect.height = height;

    button->isPressed = false;  // Initialize button as not pressed
    button->title = title;      // Set button title

    // Set button colors
    button->colorText = colorText;      // Text color
    button->color = color;              // Background color
    button->colorLine = colorLine;      // Line color
    button->colorActive = colorActive;  // Color when button is active/pressed
}

void buttonDraw(Button *button) {
    // If the button is pressed, use the active color; otherwise, use the normal color
    if (button->isPressed) {
        DrawRectangleRec(button->rect, button->colorActive);
    } else {
        DrawRectangleRec(button->rect, button->color);
    }

    // Draw the rectangle lines around the button
    DrawRectangleLinesEx(button->rect, 1, button->colorLine);

    // Draw the button title at specified position, using the specified font size and color
    DrawText(button->title, button->rect.x + 10, button->rect.y + 10, FONT_SIZE, button->colorText);
}

bool buttonCheckClick(Button *button) {
    // Check if the mouse is over the button's rectangle boundary
    if (CheckCollisionPointRec(GetMousePosition(), button->rect)) {
        // If the mouse button is pressed, mark the button as pressed
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            button->isPressed = true;
        }
        // If the mouse button is released, return true indicating a successful button click
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    } else {
        // Reset button as not pressed if above conditions are not met
        button->isPressed = false;
        return false;
    }
    button->isPressed = false;
    return false;
}
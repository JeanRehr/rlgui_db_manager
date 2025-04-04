#include <raylib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "TextBox.h"
#include "CONSTANTS.h"

typedef struct TextBox TextBox; // Declare the TextBox struct type

// Function to initialize a TextBox with the given properties
void textBoxInit(TextBox *textBox, float posX, float posY, float width, float height, const char* title, Color colorText, Color color, Color colorLine, Color colorLineActive) {
    // Initialize position and size
    textBox->rect.x = posX;
    textBox->rect.y = posY;
    textBox->rect.width = width;
    textBox->rect.height = height;

    textBox->key = 0;                  // Clear key input buffer
    textBox->currentCountChar = 0;     // Start with zero input chars
    textBox->framesCounter = 0;        // Initialize frame counter for blinking effect
    memset(textBox->input, 0, MAX_INPUT); // Clear the input buffer to avoid garbage values

    textBox->title = title;            // Set title of the textBox
    textBox->isActive = false;         // Initially, the textBox is not active

    // Set textBox colors
    textBox->colorText = colorText;        // Text color
    textBox->color = color;                // Background color
    textBox->colorLine = colorLine;        // Border color
    textBox->colorLineActive = colorLineActive; // Active border color
}

void textBoxDraw(TextBox *textBox) {
    // Draw the title above the textBox
    DrawText(textBox->title, textBox->rect.x, textBox->rect.y - FONT_SIZE, FONT_SIZE, textBox->colorText);

    // Draw the background rectangle
    DrawRectangleRec(textBox->rect, textBox->color);

    // Draw the input text inside the textBox
    DrawText(textBox->input, textBox->rect.x + 5, textBox->rect.y + 8, FONT_SIZE, textBox->colorText);

    // Active textBox gets a special border and a blinking cursor
    if (textBox->isActive) {
        DrawRectangleLinesEx(textBox->rect, 1, textBox->colorLineActive);

        // If not at max input, draw a blinking underscore
        if (textBox->currentCountChar < MAX_INPUT) {
            if (((textBox->framesCounter / 20) % 2) == 0) {
                DrawText("_", textBox->rect.x + 8 + MeasureText(textBox->input, 20), textBox->rect.y + 12, 20, textBox->colorText);
            }
        }
        textBox->framesCounter += 1;   // Increment frame counter for blinking effect
    } else {
        // Draw regular border for inactive textBox
        DrawRectangleLinesEx(textBox->rect, 1, textBox->colorLine);
        textBox->framesCounter = 0;    // Reset frame counter
    }
}

void textBoxGetInput(TextBox *textBox) {
    // Check if the textBox is active to receive input
    if (textBox->isActive) {
        textBox->key = GetKeyPressed();    // Get currently pressed key

        // Process key input
        while (textBox->key > 0) {
            // Acceptable ASCII range for input
            if ((textBox->key >= 32 && textBox->key <= 125) && (textBox->currentCountChar < MAX_INPUT)) {
                bool shouldCapitalize = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));    // Check for shift key

                // Convert letter keys based on shift key state
                if ((textBox->key >= 'A' && textBox->key <= 'Z') || (textBox->key >= 'a' && textBox->key <= 'z')) {
                    if (shouldCapitalize) {
                        textBox->input[textBox->currentCountChar] = toupper(textBox->key);   // Use uppercase
                    } else {
                        textBox->input[textBox->currentCountChar] = tolower(textBox->key);   // Use lowercase
                    }
                } else {
                    textBox->input[textBox->currentCountChar] = textBox->key;  // For non-letter keys
                }
                textBox->input[textBox->currentCountChar + 1] = '\0';  // Null terminate the string
                textBox->currentCountChar += 1;                        // Increment character count
            }

            textBox->key = GetKeyPressed();    // Get next pressed key
        }

        // Handle backspace for removing characters
        if (IsKeyDown(KEY_BACKSPACE) && textBox->currentCountChar > 0) {
            textBox->currentCountChar -= 1;    // Decrement character count
            textBox->input[textBox->currentCountChar] = '\0';    // Null terminate the string
        }
    }
}

// Function to check and set focus for the textBox using mouse input
void textBoxCheckFocus(TextBox *textBox) {
    // If the mouse is inside the rectangle and clicked, activate the textBox
    if (CheckCollisionPointRec(GetMousePosition(), textBox->rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        textBox->isActive = true;               // Activate the textBox
        SetMouseCursor(MOUSE_CURSOR_IBEAM);     // Change mouse cursor style
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        textBox->isActive = false;              // Deactivate the textBox
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);   // Reset mouse cursor style
    }
}
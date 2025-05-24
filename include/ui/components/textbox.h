/**
 * @file textbox.h
 * @brief Generic Text Input Control
 * 
 * Provides a standard text input field with labeling support. Suitable for:
 * - Free-form text entry
 * - Form inputs
 * - Anywhere textual user input is required
 */

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

#include "global/CONSTANTS.h"

/**
 * @struct textbox
 * @brief Basic text input component
 * 
 * Manages both visual presentation and edit state for a text input field.
 * Supports all printable characters with no input filtering.
 */
struct textbox {
    Rectangle bounds;      ///< Screen position and dimensions { x, y, width, height }
    char input[MAX_INPUT]; ///< Buffer storing the entered text
    const char *title;     ///< Descriptive text shown above the field (optional)
    bool edit_mode;        ///< Whether the control currently has keyboard focus
};

/**
 * @brief Initializes a new text input box
 * 
 * @param bounds Position and dimensions { x, y, width, height }
 * @param title Optional descriptive text shown above the field (may be NULL)
 * @return Preconfigured textbox instance
 * 
 * @note The input buffer is automatically zero-initialized
 * @warning Title pointer is stored directly (must remain valid for the lifetime of the textbox))
 */
struct textbox textbox_init(Rectangle bounds, const char *title);

/**
 * @brief Draws and manages the text input control
 * 
 * Handles:
 * - Visual rendering with optional title
 * - Edit mode toggling
 * - Keyboard input handling
 * 
 * @param tb Pointer to initialized textbox
 * 
 * @note Call every frame for proper interaction
 */
void textbox_draw(struct textbox *tb);

#endif // TEXTBOX_H

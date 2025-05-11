/**
 * @file floatbox.h
 * @brief Floating-Point Input Control
 * 
 * Provides a text input field specialized for floating-point values with:
 * - Built-in numeric validation
 * - Label support
 * - Edit mode management
 */

#ifndef FLOATBOX_H
#define FLOATBOX_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

#include "CONSTANTS.h"

/**
 * @struct floatbox
 * @brief Floating-point input component
 * 
 * Combines text input with parsed float value storage, maintaining
 * both string and numeric representations.
 */
struct floatbox {
    Rectangle bounds;           ///< Screen position and dimensions { x, y, width, height }
    const char *title;          ///< Descriptive title shown above the field
    char text_input[MAX_INPUT]; ///< String buffer for textual representation
    float value;                ///< Parsed float value
    bool edit_mode;             ///< Whether the control currently has focus
};

/**
 * @brief Initializes a new float input box
 * 
 * @param bounds Position and dimensions { x, y, width, height }
 * @param title Descriptive text shown above the field (required)
 * @return Preconfigured floatbox with:
 *         - value initialized to 0.0f
 *         - edit_mode set to false
 *         - text_input buffer zeroed
 * 
 * @warning Title pointer is stored directly (must remain valid)
 */
struct floatbox floatbox_init(Rectangle bounds, const char *title);

/**
 * @brief Draws and manages the float input control
 * 
 * Handles:
 * - Title/label rendering
 * - Edit mode toggling
 * - String-to-float conversion
 * - Input validation
 * 
 * @param fb Pointer to initialized floatbox
 * 
 * @note Call every frame for proper interaction
 * @warning Uses GuiValueBoxFloat which modifies both text_input and value
 */
void floatbox_draw(struct floatbox *fb);

#endif // FLOATBOX_H

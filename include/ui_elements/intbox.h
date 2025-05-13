/**
 * @file intbox.h
 * @brief Bounded Integer Input Control
 * 
 * Provides a numeric input field that enforces minimum/maximum value constraints.
 * Uses RayGUI's GuiValueBox internally with enhanced label handling.
 */

#ifndef INTBOX_H
#define INTBOX_H

#include <stdbool.h>

#include <external/raylib/raylib.h>

/**
 * @struct intbox
 * @brief Integer input box with value range enforcement
 * 
 * Combines a labeled input control with automatic value clamping.
 */
struct intbox {
    Rectangle bounds;  ///< Screen position and dimensions { x, y, width, height }
    const char *title; ///< Descriptive text shown above the field
    int input;         ///< Current numeric value (guaranteed between min_val/max_val)
    int min_val;       ///< Minimum allowed value (inclusive)
    int max_val;       ///< Maximum allowed value (inclusive)
    bool edit_mode;    ///< Whether the control currently has keyboard focus
};

/**
 * @brief Initializes a bounded integer input box
 * 
 * @param bounds Position and dimensions { x, y, width, height }
 * @param title Descriptive text shown above the field (required)
 * @param min_val Minimum allowed value (inclusive)
 * @param max_val Maximum allowed value (inclusive)
 * @return Preconfigured intbox instance
 * 
 * @note Initial value is set to 0 (clamped to valid range)
 * @warning Title pointer is stored directly (must remain valid for the lifetime of the intbox)).
 *          If min_val > max_val leads to unusable intbox, this is not asserted.
 */
struct intbox intbox_init(Rectangle bounds, const char *title, const int min_val, const int max_val);

/**
 * @brief Draws and manages the integer input control
 * 
 * Handles:
 * - Adaptive title sizing (expands if text is wider than box)
 * - Value clamping enforcement
 * - Edit mode toggling
 * - Keyboard/button input handling
 * 
 * @param ib Pointer to initialized intbox
 * 
 * @note Call every frame for proper interaction
 * @warning Uses GuiValueBox which may modify input value directly
 */
void intbox_draw(struct intbox *ib);

#endif // INTBOX_H

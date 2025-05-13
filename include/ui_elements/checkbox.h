/**
 * @file checkbox.h
 * @brief Checkbox Control with Label
 *
 * Provides a labeled checkbox component for boolean options and toggles.
 * Wraps RayGUI's GuiCheckBox with automatic label positioning.
 */

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <stdbool.h>

#include <external/raylib/raylib.h>

/**
 * @struct checkbox
 * @brief Checkbox component with title
 *
 * Combines a checkbox with its descriptive label at the top and
 * manages their visual relationship and interaction state.
 */
struct checkbox {
    Rectangle bounds;  ///< Position and dimensions of the checkbox { x, y, width, height }
    const char *title; ///< Descriptive text shown at the top of the checkbox
    bool checked;      ///< Current toggle state (true = checked, false = unchecked)
};

/**
 * @brief Initializes a new checkbox
 *
 * @param bounds Position and dimensions of the checkbox { x, y, width, height }
 * @param title Descriptive text shown with the checkbox (required)
 * @return Preconfigured checkbox instance with:
 * 
 * - checked state initialized to false
 * 
 * - Title pointer stored (must remain valid)
 *
 * @note The label will be automatically positioned to the top of the checkbox
 * @warning Title pointer is stored directly (must remain valid for the lifetime of the checkbox)
 */
struct checkbox checkbox_init(Rectangle bounds, const char *title);

/**
 * @brief Draws and manages the checkbox control
 *
 * Handles:
 * 
 * - Label rendering with automatic positioning
 * 
 * - Checkbox visual states
 * 
 * - Mouse interaction
 * 
 * - Toggle state changes
 *
 * @param cb Pointer to initialized checkbox
 * @return 1 if state changed this frame, 0 otherwise
 *
 * @note Call every frame for proper interaction
 * @note Return value can be used to detect user toggles
 */
int checkbox_draw(struct checkbox *cb);

#endif // CHECKBOX_H

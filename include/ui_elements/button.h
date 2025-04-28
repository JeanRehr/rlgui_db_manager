/**
 * @file button.h
 * @brief Smart Button Control
 *
 * Provides an auto-sizing button component that intelligently adjusts its width
 * to fit the button text while maintaining consistent interaction behavior.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

/**
 * @struct button
 * @brief Button component with automatic sizing
 *
 * Manages both visual presentation and interaction state for a GUI button,
 * automatically adjusting width to accommodate the button text.
 */
struct button {
    Rectangle bounds;  ///< Position and dimensions {x,y,width,height}
    const char *title; ///< Text displayed on the button (required)
};

/**
 * @brief Initializes a new auto-sizing button
 *
 * @param bounds Base position and dimensions { x, y, width, height }
 * @param title Text to display on the button (required)
 * @return Configured button instance with:
 *         - Width automatically adjusted for text if needed
 *         - Height preserved from input bounds
 *
 * @note Expands width if text is wider than initial bounds
 * @warning Title pointer is stored directly (must remain valid)
 * @warning Minimum width is preserved (won't shrink below initial bounds.width)
 */
struct button button_init(Rectangle bounds, const char *title);

/**
 * @brief Draws and updates the button
 *
 * Handles:
 * - Visual rendering with automatic state coloring
 * - Mouse interaction
 * - Press state detection
 *
 * @param button Pointer to initialized button
 * @return 1 if button was pressed this frame, 0 otherwise
 *
 * @note Call every frame for proper interaction
 * @note Return value can be used to trigger button actions
 */
int button_draw_updt(struct button *button);

#endif // BUTTON_H

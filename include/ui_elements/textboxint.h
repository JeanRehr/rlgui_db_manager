/**
 * @file textboxint.h
 * @brief Integer-Only Input Control
 *
 * Provides a text input field that enforces numeric-only input while
 * maintaining string storage for leading zero support. Useful for:
 * - Numeric IDs (e.g. CPF)
 * - Any integer value requiring text box flexibility
 */

#ifndef TEXTBOXINT_H
#define TEXTBOXINT_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

#include "CONSTANTS.h"

/**
 * @struct textboxint
 * @brief Integer input text box component
 *
 * Maintains both the visual representation and validation state
 * for an integer-only input field.
 */
struct textboxint {
    Rectangle bounds;      ///< Screen position and dimensions { x, y, width, height }
    char input[MAX_INPUT]; ///< String buffer storing numeric characters
    const char *title;     ///< Descriptive title shown above the field (optional)
    bool edit_mode;        ///< Whether the control currently has focus
};

/**
 * @brief Initializes an integer input text box
 *
 * @param bounds Position and dimensions { x, y, width, height }
 * @param title Optional descriptive text shown above the field (may be NULL)
 * @return Preconfigured textboxint instance
 *
 * @note The input buffer is automatically zero-initialized
 * @warning Title pointer is stored directly (must remain valid)
 */
struct textboxint textboxint_init(Rectangle bounds, const char *title);

/**
 * @brief Draws and manages the integer input control
 *
 * Handles:
 * - Visual rendering with title
 * - Edit mode toggling
 * - Real-time input filtering
 *
 * @param tbi Pointer to initialized textboxint
 *
 * @note Call every frame for proper interaction
 * @warning Uses filter_integer_input() from utils_fn.h on implementation to enforce numeric-only input
 */
void textboxint_draw(struct textboxint *tbi);

#endif // TEXTBOXINT_H

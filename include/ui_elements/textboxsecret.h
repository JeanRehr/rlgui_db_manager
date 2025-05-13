/**
 * @file textboxsecret.h
 * @brief Secure Password Input Control
 *
 * Provides a password entry field with toggleable visibility that:
 * - Masks input by default (showing ••••)
 * - Allows temporary reveal of entered text
 * - Handles secure input buffering
 */

#ifndef TEXTBOXSECRET_H
#define TEXTBOXSECRET_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

#include "CONSTANTS.h"

/**
 * @struct textboxsecret
 * @brief Secure password input control
 *
 * Encapsulates all state for a password entry field with visibility toggle.
 * Uses a modified RayGUI's GuiTextInputBox internally while providing additional labeling.
 */
struct textboxsecret {
    Rectangle bounds;
    const char *title;
    char input[MAX_INPUT];
    bool secret_view; // Toggle to display the actual text
};

/**
 * @brief Initializes a new secure text box
 *
 * @param bounds Screen rectangle defining position/size {x,y,width,height}
 * @param title Descriptive text shown above the field (may be NULL)
 * @return Fully initialized textboxsecret instance
 *
 * @note The input buffer is automatically zero-initialized
 * @warning Title pointer is stored directly (must remain valid for the lifetime of the textboxsecret))
 */
struct textboxsecret textboxsecret_init(Rectangle bounds, const char *title);

/**
 * @brief Draws the password input control
 *
 * Renders both the title and interactive text box. Handles:
 * - Input masking (••••) when secret_view=false
 * - Text display when secret_view=true
 * - Mouse interactions and focus states
 *
 * @param tbs Pointer to initialized textboxsecret
 *
 * @note Call every frame for interactive behavior
 * @warning Requires GuiLock()/GuiUnlock() if using raygui in threaded context
 */
void textboxsecret_draw(struct textboxsecret *tbs);

#endif // TEXTBOXSECRET_H

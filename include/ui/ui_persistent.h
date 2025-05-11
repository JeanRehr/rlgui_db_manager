/**
 * @file ui_persistent.h
 * @brief Persistent UI Elements Manager
 *
 * Manages UI elements that remain visible across all application screens,
 * including user session controls and global style settings.
 */

#ifndef UI_PERSISTENT_H
#define UI_PERSISTENT_H

#include "app_state.h"
#include "ui_elements/button.h"
#include "ui_elements/dropdownbox.h"
#include "user.h"

/**
 * @struct ui_persistent
 * @brief Persistent UI elements container
 *
 * Contains all UI elements that should remain visible regardless
 * of the current application screen state.
 */
struct ui_persistent {
    struct button logout_butn;            ///< Session logout button
    struct dropdownbox ddb_style_options; ///< Style selector
    int prev_active_style;                ///< Number to check for any changes in the active option style
    Rectangle statusbar_bounds;           ///< Status bar display area
};

/**
 * @brief Initializes persistent UI elements
 *
 * Sets up all persistent elements with default positions and values.
 * Should be called once during application startup.
 *
 * @param ui Pointer to ui_persistent struct to initialize
 */
void ui_persistent_init(struct ui_persistent *ui);

/**
 * @brief Draws and updates persistent UI elements
 *
 * Handles rendering and interaction for all persistent elements.
 * Should be called every frame after the current screen's draw function.
 *
 * @param ui Pointer to initialized ui_persistent struct
 * @param current_user Pointer to current user session data
 * @param state Pointer to application state (may be modified for logout)
 */
void ui_persistent_draw(struct ui_persistent *ui, struct user *current_user, enum app_state *state);

/**
 * @brief Updates persistent ui
 *
 * Right now only serves the purpose to update the style
 *
 * @param ui Pointer to ui_persistent struct which has the style selector
 * 
 */
void ui_persistent_updt(struct ui_persistent *ui);

/**
 * @brief Updates persistent element positions
 *
 * Adjusts UI element positions based on current window dimensions.
 * Should be called when the window is resized.
 *
 * @param ui Pointer to ui_persistent struct to update
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_persistent_updt_pos(struct ui_persistent *ui);

#endif // UI_PERSISTENT_H

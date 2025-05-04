/**
 * @file ui_base.h
 * @brief Base UI Screen Structure
 */

#ifndef UI_BASE_H
#define UI_BASE_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"

// Forward declaration
struct ui_base;

// Function pointer typedefs
typedef void (*render_fn)(struct ui_base *base, enum app_state *state, enum error_code *error, database *db);

typedef void (*handle_buttons_fn)(struct ui_base *base, enum app_state *state, enum error_code *error, database *db);

typedef void (*handle_warning_msg_fn)(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *db
);

typedef void (*update_positions_fn)(struct ui_base *base);

typedef void (*clear_fields_fn)(struct ui_base *base);

struct ui_base {
    const char* type_name; ///< name of the struct that is using the base interface

    /**
     * @brief Draws and handles interactions for the UI screen
     * 
     * @param base Pointer to the base (interface) UI, can be safely cast to any other UI
     * @param state Pointer to the current app_state screen (may be modified)
     * @param error Pointer to the current error code (may be modified)
     * @param db Pointer to the database for operations
     * 
     * @note Follows raygui's immediate-mode pattern where drawing 
     *       and interaction handling are combined in one operation
     * 
     */
    render_fn render;

    /**
     * @brief Manages button logic handling
     * 
     * @param base Pointer to the base (interface) UI, can be safely cast to any other UI
     * @param state Pointer to the current app_state screen (may be modified)
     * @param error Pointer to the current error code (may be modified)
     * @param db Pointer to the database for operations
     * 
     * @note Follows raygui's immediate-mode pattern where drawing and interaction handling
     *       are combined in one operation, so this will also draw buttons, every screen
     *       will have either a back button or login button (ui login screen)
     * 
     */
    handle_buttons_fn handle_buttons;

    /**
     * @brief Manages button logic handling
     * 
     * @param base Pointer to the base (interface) UI, can be safely cast to any other UI
     * @param state Pointer to the current app_state screen (may be modified)
     * @param error Pointer to the current error code (may be modified)
     * @param db Pointer to the database for operations
     * 
     * @note Every screen needs to show at least 1 message, but a default is provided if this is not true.
     *       Some warning messages will do database operations, such as confirming for deletion, update passwd, etc,
     *       that's the reason for the db pointer
     * 
     */
    handle_warning_msg_fn handle_warning_msg;

    /**
     * @brief Updates screen element positions
     * Adjusts UI elements based on current window dimensions.
     * 
     * @param base Pointer to the base (interface) UI, can be safely cast to any other UI
     * 
     * @note If any ui element is initialized with window_width or window_height
     *       in their bounds, they must be updated here
     * 
     */
    update_positions_fn update_positions;

    /**
     * @brief Clear any necessary fields in a screen
     * 
     * @param base Pointer to the base (interface) UI, can be safely cast to any other UI
     * 
     */
    clear_fields_fn clear_fields;
};

/**
 * @brief Base constructor for default behavior
 * 
 * @param base Pointer to the base struct
 * 
 * @note Does not implement anything useful, just makes it so that anything not implemented in a derived struct
 *       does not lead to a runtime crash
 */
void ui_base_init_defaults(struct ui_base *base, const char *type_name);

#endif // UI_BASE_H
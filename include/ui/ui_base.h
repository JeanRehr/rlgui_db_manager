/**
 * @file ui_base.h
 * @brief Base UI interface for all screens (polymorphic behavior)
 * 
 * Provides function pointers for common UI operations:
 * - Rendering and interaction
 * - Dynamic positioning
 * - Resource cleanup
 * 
 * @note All derived screens (screens that either embed or has a pointer to ui_base) must implement these callbacks.
 */
#ifndef UI_BASE_H
#define UI_BASE_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"

// Forward declaration
struct ui_base;

/**
 * @brief Function pointer for rendering and handling UI interactions.
 * @param base  Base UI struct (castable to derived screens)
 * @param state Application state (may be modified)
 * @param error Error tracking (may be modified)
 * @param db    Database connection for UI operations
 * 
 * @warning Immediate-mode rendering handles drawing and input one frame at a time
 * 
 * @note Handling of buttons and message warnings should be done here
 */
typedef void (*render_fn)(struct ui_base *base, enum app_state *state, enum error_code *error, database *db);

/**
 * @brief Function pointer for button state management.
 * @note Combines drawing and interaction handling (immediate-mode GUI pattern).
 */
typedef void (*handle_buttons_fn)(struct ui_base *base, enum app_state *state, enum error_code *error, database *db);

/**
 * @brief Function pointer for warning/confirmation dialogs.
 * @note May trigger database operations (e.g., deletions, updates).
 */
typedef void (*handle_warning_msg_fn)(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *db
);

/**
 * @brief Function pointer for updating UI element positions.
 * @warning Call only on window resize events.
 */
typedef void (*update_positions_fn)(struct ui_base *base);

/**
 * @brief Function pointer for resetting input fields.
 */
typedef void (*clear_fields_fn)(struct ui_base *base);

/**
 * @brief Function pointer for freeing screen-specific resources.
 * @details Must deallocate any memory owned by derived screens (e.g., buffers, dynamic UI elements).
 */
typedef void (*cleanup_fn)(struct ui_base *base);

/**
 * struct ui_vtable
 * brief Virtual function table defining UI screen behavior.
 *
 * This structure contains function pointers that define the behavior of UI screens.
 * Each UI screen type has a single shared vtable instance containing implementations
 * specific to that screen type. This enables polymorphic behavior while maintaining
 * type safety and memory efficiency.
 * 
 * I have tested a vtable approach, but this doesn't really add anything useful
 * aside from shared memory for instances of the same type
 * 
 * vtable enforces everything is implemented, but C doesn't warn you if something isn't implemented
 * and it silently sets the function pointer not implemented to null
 * 
 * the vtable overrides everything set in the ui_base_init_defaults,
 * as it assigns everything in the function pointer vtable at the same time
 * 
 * so using direct function pointers in ui_base provides more usefulness and safety
 * (only because the compiler doesn't warn for unimplemented functions and
 * there isn't an implicit constructor and destructor in c)
 * @code{.c}
 * struct ui_vtable {
 *     render_fn render;                         ///< Combined render/interaction handler
 *     handle_buttons_fn handle_buttons;         ///< Button state manager
 *     handle_warning_msg_fn handle_warning_msg; ///< Dialog handler
 *     update_positions_fn update_positions;     ///< Layout updater (window resize)
 *     clear_fields_fn clear_fields;             ///< Input field reset
 *     cleanup_fn cleanup;                       ///< Resource deallocator
 *     (*bla)(struct ui_base *base); // testing adding a function to see if any warning happens
 * };
 * @endcode
 */

/**
 * @struct ui_base
 * @brief Base structure for polymorphic UI screen implementation.
 *
 * Provides the foundation for UI screen polymorphism through a virtual table.
 * All UI screen structures must:
 * 
 * 1. Embed this struct as their first member
 * 
 * 2. Initialize the function pointers with their own implementation
 *
 * @note Use ui_base_init_defaults() to set up a screen with default no-op implementations.
 * 
 * To add a new behavior:
 * 
 * 1. Add the field to struct ui_base
 * 
 * 2. Implement a default version in ui_base.c
 * 
 * 3. Add it to ui_base_init_defaults()
 * 
 * 4. Only implement and override in screens that need this new behavior
 * 
 * Example:
 * @code{.c}
 * struct ui_login {
 *     struct ui_base base;  // Must be first member
 *     // Login-specific fields...
 *     // -- snip --
 * };
 *
 * void ui_login_init(...) {
 *     // Initialize base to default (prevent runtime crashes)
 *     ui_base_init_defaults(&ui->base, "UI Login")
 *     
 *     // Override methods
 *     ui->base.render = ui_login_render;
 *     ui->base.handle_buttons = ui_login_handle_buttons;
 *     // -- snip --
 * }
 * @endcode
 * 
 */
struct ui_base {
    render_fn render;                         ///< Combined render/interaction handler
    handle_buttons_fn handle_buttons;         ///< Button state manager
    handle_warning_msg_fn handle_warning_msg; ///< Dialog handler
    update_positions_fn update_positions;     ///< Layout updater (window resize)
    clear_fields_fn clear_fields;             ///< Input field reset
    cleanup_fn cleanup;                       ///< Resource deallocator

    const char *type_name; ///< Name of the derived screen type (for debugging)
};

/**
 * @brief Initializes a ui_base struct with default no-op implementations.
 * 
 * @param base       Uninitialized base struct
 * @param type_name  Identifier for the derived screen (e.g. ui_login)
 * 
 * @warning Derived screens MUST override these defaults with their own implementations.
 */
void ui_base_init_defaults(struct ui_base *base, const char *type_name);

#endif // UI_BASE_H

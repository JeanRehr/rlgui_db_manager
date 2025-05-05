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
 * @brief Base UI structure for polymorphic screen behavior.
 * 
 * All derived screens must:
 * 1. Embed this struct as their first member.
 * 2. Implement the function pointers in their init function.
 * 
 */
struct ui_base {
    const char *type_name; ///< Name of the derived screen type (for debugging)

    render_fn render;                         ///< Combined render/interaction handler
    handle_buttons_fn handle_buttons;         ///< Button state manager
    handle_warning_msg_fn handle_warning_msg; ///< Dialog handler
    update_positions_fn update_positions;     ///< Layout updater (window resize)
    clear_fields_fn clear_fields;             ///< Input field reset
    cleanup_fn cleanup;                       ///< Resource deallocator
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
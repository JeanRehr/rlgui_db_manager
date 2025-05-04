/**
 * @file ui_main_menu.h
 * @brief Main Menu Screen Management
 *
 * Handles the presentation and interaction of the application's
 * main menu navigation interface.
 */

#ifndef UI_MAIN_MENU_H
#define UI_MAIN_MENU_H

/**
 * @enum main_menu_screen_flags
 * @brief State flags for the main menu
 *
 * Tracks various states and validation results for the main menu.
 */
enum main_menu_screen_flags {
    FLAG_MAIN_MENU_WARN_NOT_ADMIN = 1 << 0, ///< Flag for warning current user is not admin
};

/**
 * @struct ui_main_menu
 * @brief Main menu screen UI components
 *
 * Contains all interactive elements for the main menu navigation.
 */
#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "ui/ui_base.h"
#include "ui_elements/button.h"
#include "user.h"

// To manage the state of the main menu screen
struct ui_main_menu {
    struct ui_base base; ///< Base ui methods/functionality

    struct button reg_resident_butn; ///< Button for resident management
    struct button reg_food_butn;     ///< Button for food inventory management
    struct button create_user_butn;  ///< Button for the create user screen

    struct user *current_user; ///< Pointer to the current user for checking admin

    enum main_menu_screen_flags flag; ///< Flags for the struct
};

/**
 * @brief Initializes main menu UI elements
 *
 * Sets up all menu buttons with default positions and labels.
 * Should be called when transitioning to the main menu screen.
 *
 * @param ui Pointer to ui_main_menu struct to initialize
 * @param current_user Pointer to the current user to set the context up
 */
void ui_main_menu_init(struct ui_main_menu *ui, struct user *current_user);

/**
 * @brief Main menu screen rendering and interaction handling.
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state (modified on navigation)
 * @param error Pointer to error code
 * @param user_db Pointer to the user database for querying if the current user is really admin
 * 
 * @warning Should be called through the base interface
 */
void ui_main_menu_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

/**
 * @brief Handle button drawing and logic.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * 
 * @warning Should be called through the base interface
 */
void ui_main_menu_handle_buttons(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

/**
 * @brief Manages warning message display and response handling.
 * 
 * Shows appropriate warning messages based on current user admin status,
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * 
 * @warning Should be called through the base interface
 */
void ui_main_menu_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

/**
 * @brief Updates main menu element's screen positions
 *
 * Adjusts UI elements based on current window dimensions.
 * Maintains proper component layout during window resizing.
 *
 * @param base Pointer to base UI (interface) structure (can be safely cast to any ui*)
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 * 
 * @warning Should be called through the base interface
 * 
 */
void ui_main_menu_updt_pos(struct ui_base *base);

/**
 * @brief Clear fields of the ui_main_menu
 *
 * @param base Pointer to base UI (interface) structure (can be safely cast to any ui*)
 * 
 * @note It is necessary to clear any sensitive data on implementation
 * 
 * @warning Should be called through the base interface
 * 
 */
void ui_main_menu_clear_fields(struct ui_base *base);

#endif // UI_MAIN_MENU_H
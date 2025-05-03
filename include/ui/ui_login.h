/**
 * @file ui_login.h
 * @brief Login Screen Management
 *
 * Handles user authentication flow including:
 * - Username/password input
 * - Credential validation
 * - Password reset functionality if flagged
 * - Secure session initiation
 */

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "ui/ui_base.h"
#include "ui_elements/button.h"
#include "ui_elements/textbox.h"
#include "ui_elements/textboxsecret.h"
#include "user.h"

/**
 * @enum login_screen_flags
 * @brief State flags for login screen operations
 *
 * Tracks various authentication states and validation results.
 * Used for controlling UI feedback and workflow.
 */
enum login_screen_flags {
    FLAG_LOGIN_DONE = 1 << 0,      ///< Authentication completed successfully
    FLAG_USER_NOT_EXISTS = 1 << 1, ///< Specified username not found
    FLAG_WRONG_PASSWD = 1 << 2,    ///< Incorrect password provided
    FLAG_USERNAME_EMPTY = 1 << 3,  ///< Username field empty
    FLAG_PASSWD_EMPTY = 1 << 4,    ///< Password field empty
    FLAG_PASSWD_RESET = 1 << 5     ///< Password reset required
};

/**
 * @struct ui_login
 * @brief Login screen UI components and state
 *
 * Manages all interactive elements for the authentication interface.
 */
struct ui_login {
    struct ui_base base;               ///< Base ui methods/functionality
    struct user *current_user;         ///< Pointer to the current user struct
    struct textbox tb_username;        ///< Username input field
    struct textboxsecret tbs_password; ///< Secure password input field
    struct button butn_login;          ///< Authentication submission button
    enum login_screen_flags flag;      ///< Current authentication state flags
};

/**
 * @brief Initializes login screen components
 *
 * Sets up all UI elements with default positions and security states.
 * Everything is deinitialized/set to zero once after the login screen.
 *
 * @param ui Pointer to ui_login struct to initialize (knows concrete type)
 * @param current_user Pointer to the current user struct, as it is on login, it will be zero
 *                     and filled when the user logs in
 */
void ui_login_init(struct ui_login *ui, struct user *current_user);

/**
 * @brief Login screen rendering and interaction handling.
 *
 * Handles:
 * - Credential input rendering
 * - Authentication workflow
 * - Secure session initiation
 * - Error feedback
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to ui_login*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * @param current_user Pointer to current user session struct
 * 
 * @warning Should be called through the base interface
 */
void ui_login_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

/**
 * @brief Handle button drawing and logic.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to ui_login*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * 
 * @warning Should be called through the base interface
 */
void ui_login_handle_buttons(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

/**
 * @brief Manages warning message display and response handling.
 * 
 * Shows appropriate warning messages based on login attempt flags,
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to ui_login*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * 
 * @warning Should be called through the base interface
 */
void ui_login_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

/**
 * @brief Updates login screen positions
 *
 * Adjusts UI elements based on current window dimensions.
 * Maintains proper component layout during window resizing.
 *
 * @param base Pointer to base UI (interface) structure (can be safely cast to ui_login*)
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 * 
 * @warning Should be called through the base interface
 * 
 */
void ui_login_updt_pos(struct ui_base *base);

/**
 * @brief Clear fields of the ui_login
 *
 * @param base Pointer to base UI (interface) structure (can be safely cast to ui_login*)
 * 
 * @note It is necessary to clear any sensitive data on implementation
 * 
 * @warning Should be called through the base interface
 * 
 */
void ui_login_clear_fields(struct ui_base *base);

#endif // UI_LOGIN_H
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
 * @param ui Pointer to ui_login struct to initialize
 */
void ui_login_init(struct ui_login *ui);

/**
 * @brief Draws and manages login screen
 *
 * Handles:
 * - Credential input rendering
 * - Authentication workflow
 * - Secure session initiation
 * - Error feedback
 *
 * @param ui Pointer to initialized ui_login struct
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 * @param current_user Pointer to current user session struct
 */
void ui_login_draw(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

/**
 * @brief Updates login screen positions
 *
 * Adjusts UI elements based on current window dimensions.
 * Maintains proper component layout during window resizing.
 *
 * @param ui Pointer to ui_login struct to update
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_login_updt_pos(struct ui_login *ui);

#endif // UI_LOGIN_H
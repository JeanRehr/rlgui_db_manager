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

#include "ui/screens/ui_base.h"
#include "ui/components/button.h"
#include "ui/components/textbox.h"
#include "ui/components/textboxsecret.h"
#include "entities/user.h"

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

#endif // UI_LOGIN_H

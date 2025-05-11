/**
 * @file ui_create_user.h
 * @brief User Creation Management Screen
 *
 * Provides interface for managing creation of new users, including:
 * - Adding new admin
 * - Updating username
 * - Updating password
 * - @todo add more
 */

#ifndef UI_CREATE_USER_H
#define UI_CREATE_USER_H

#include "ui/ui_base.h"
#include "ui_elements/button.h"
#include "ui_elements/checkbox.h"
#include "ui_elements/textbox.h"

/**
 * @enum create_user_screen_flags
 * @brief State flags for create user management operations
 *
 * Tracks various states and validation results for user creation management.
 */
enum create_user_screen_flags {
    FLAG_CREATE_USER_OPERATION_DONE = 1 << 0, ///< Operation done
    FLAG_CREATE_USER_USERNAME_EMPTY = 1 << 0, ///< Username is empty during user creation
};

/**
 * @struct ui_create_user
 * @brief Complete user creation screen state
 *
 * Contains all UI elements and state information needed to manage
 * creation of users.
 */
struct ui_create_user {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_username; ///< Textbox for the username of the new user

    struct checkbox cb_is_admin; ///< Checkbox for the is_admin attribute

    struct button butn_create_user;    ///< Button for the creation of a user
    struct button butn_reset_password; ///< Button for an admin to reset a user's password
    struct button butn_back;           ///< Navigation back button
};

/**
 * @brief Initializes user management screen
 *
 * Sets up all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_create_user struct to initialize
 */
void ui_create_user_init(struct ui_create_user *ui);

#endif // UI_CREATE_USER_H

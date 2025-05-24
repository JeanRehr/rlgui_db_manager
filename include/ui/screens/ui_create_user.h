/**
 * @file ui_create_user.h
 * @brief User Creation Management Screen
 *
 * Provides interface for managing creation of new users, including:
 *
 * - Adding new users
 *
 * - Update admin status
 *
 * - Resetting password
 *
 * - Delete a user
 *
 * - View all users
 */

#ifndef UI_CREATE_USER_H
#define UI_CREATE_USER_H

#include "ui/screens/ui_base.h"
#include "ui/components/button.h"
#include "ui/components/checkbox.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/components/textboxint.h"

/**
 * @enum create_user_screen_flags
 * @brief State flags for create user management operations
 *
 * Tracks various states and validation results for user creation management.
 */
enum create_user_screen_flags {
    FLAG_CREATE_USER_OPERATION_DONE = 1 << 0,          ///< Operation done
    FLAG_CREATE_USER_USERNAME_EMPTY = 1 << 1,          ///< Username is empty during user creation
    FLAG_CREATE_USER_CPF_EMPTY = 1 << 2,               ///< CPF is empty during user creation
    FLAG_CREATE_USER_CPF_EXISTS = 1 << 3,              ///< CPF already exists during user creation
    FLAG_CREATE_USER_CPF_NOT_VALID = 1 << 4,           ///< CPF not valid during user creation
    FLAG_CREATE_USER_PHONE_NUMBER_WRONG = 1 << 5,      ///< Phone number not expected format
    FLAG_CREATE_USER_USERNAME_ALREADY_EXISTS = 1 << 6, ///< Username already exists
    FLAG_CREATE_USER_USERNAME_NOT_EXISTS = 1 << 7,     ///< User doesn't exists
    FLAG_CREATE_USER_CONFIRM_DELETE = 1 << 8,          ///< Confirm user deletion
    FLAG_CREATE_USER_ADMIN_TEMPER = 1 << 9,            ///< default admin being tempered with
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

    struct textbox tb_username;         ///< Textbox for the username of the new user
    struct textboxint tbi_cpf;          ///< Textboxint for the cpf of the new user
    struct textboxint tbi_phone_number; ///< Textboxint for the phone number of the new user

    struct checkbox cb_is_admin; ///< Checkbox for the is_admin attribute

    struct button butn_create_user;     ///< Button for the creation of a user
    struct button butn_reset_password;  ///< Button for an admin to reset a user's password
    struct button butn_update_adm_stat; ///< Button for an admin to update a user's admin status
    struct button butn_delete;          ///< Button for deleting a user from the app
    struct button butn_get_all;         ///< Button for getting all users from database
    struct button butn_back;            ///< Navigation back button

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the users's database
    char *str_table_content;          ///< The content of the users's database (MUST BE FREED IF ALLOCATED)

    enum create_user_screen_flags flag;
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

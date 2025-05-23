/**
 * @file ui_settings.h
 * @brief Settings Management of the user
 *
 * Manages all UI elements and state for the settings screens,
 * including form input, database operations.
 * It will update info on the ucrrently logged in user.
 */

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "ui/ui_base.h"
#include "ui_elements/button.h"
#include "ui_elements/dropdownbox.h"
#include "ui_elements/textbox.h"
#include "ui_elements/textboxint.h"
#include "user.h"

/**
 * @enum settings_screen_flags
 * @brief State flags for settings screen operations
 *
 * Bitmask flags that track various states and validation results
 * for the settings screen.
 */
enum settings_screen_flags {
    FLAG_SETTINGS_OPERATION_DONE = 1 << 0,     ///< Submission completed successfully
    FLAG_SETTINGS_USERNAME_EXISTS = 1 << 1,    ///< Chosen username already exists
    FLAG_SETTINGS_CPF_EXISTS = 1 << 2,         ///< Chosen CPF already exists
    FLAG_SETTINGS_CPF_NOT_VALID = 1 << 3,      ///< CPF not valid format (11 digits)
    FLAG_SETTINGS_PHONE_NUMBER_WRONG = 1 << 4, ///< Phone number not valid format (13 digits)
    FLAG_SETTINGS_ADMIN_TEMPER = 1 << 5,       ///< Temper of admin username
};

/**
 * @struct ui_settings
 * @brief Complete settings screen state
 *
 * Contains all UI elements and state information needed to manage
 * the settings screen.
 */
struct ui_settings {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_new_username;         ///< New username to set for the currently logged in user
    struct textboxint tbi_new_phone_number; ///< New new phone number to set for the currently logged in user
    struct textboxint tbi_new_cpf;          ///< New new cpf to set for the currently logged in user

    struct dropdownbox ddb_style_options; ///< Style selector
    int prev_active_style;                ///< Number to check for any changes in the active option style

    struct button butn_back;           ///< Return to previous screen
    struct button butn_submit;         ///< Submit updated info to the database
    struct button butn_reset_password; ///< Reset logged-in user password

    Rectangle panel_bounds; ///< Panel bounds to display current user info

    struct user *current_user; ///< Pointer to the currently logged in user

    enum settings_screen_flags flag; ///< Current screen state flags
};

/**
 * @brief Initializes the settings screen
 *
 * Sets up base interface overrides and all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_settings struct to initialize
 */
void ui_settings_init(struct ui_settings *ui, struct user *current_user);

#endif // UI_SETTINGS_H

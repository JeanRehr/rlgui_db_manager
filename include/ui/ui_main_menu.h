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

#include "ui/ui_base.h"
#include "ui_elements/button.h"
#include "user.h"


/**
 * @struct ui_main_menu
 * @brief Main menu screen UI components
 *
 * Contains all interactive elements for the main menu navigation.
 */
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

#endif // UI_MAIN_MENU_H

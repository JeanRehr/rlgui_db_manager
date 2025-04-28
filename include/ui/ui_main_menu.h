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
 * @struct ui_main_menu
 * @brief Main menu screen UI components
 *
 * Contains all interactive elements for the main menu navigation.
 */
#include "app_state.h"
#include "error_handling.h"
#include "ui_elements/button.h"

// To manage the state of the main menu screen
struct ui_main_menu {
    struct button reg_resident_butn; ///< Button for resident management
    struct button reg_food_butn;     ///< Button for food inventory management
};

/**
 * @brief Initializes main menu UI elements
 *
 * Sets up all menu buttons with default positions and labels.
 * Should be called when transitioning to the main menu screen.
 *
 * @param ui Pointer to ui_main_menu struct to initialize
 */
void ui_main_menu_init(struct ui_main_menu *ui);

/**
 * @brief Draws and updates main menu screen
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param ui Pointer to initialized ui_main_menu struct
 * @param state Pointer to application state (modified on navigation)
 * @param error Pointer to error code
 */

/**
 * @brief Updates main menu element positions
 *
 * Adjusts UI element positions based on current window dimensions.
 * Should be called when the window is resized.
 *
 * @param ui Pointer to ui_main_menu struct to update
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_main_menu_updt_pos(struct ui_main_menu *ui);

#endif // UI_MAIN_MENU_H
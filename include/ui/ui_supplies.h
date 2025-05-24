/**
 * @file ui_supplies.h
 * @brief Supplies Screen Management
 *
 * Handles the presentation and interaction of the application's
 * supplies management interface.
 */

#ifndef UI_SUPPLIES_H
#define UI_SUPPLIES_H

/**
 * @enum supplies_screen_flags
 * @brief State flags for the supplies screen
 *
 * Tracks various states and validation results for the supplies screen.
 */
enum supplies_screen_flags {
    FLAG_SUPPLIES_OPERATION_DONE = 1 << 0, ///< Operation done
};

#include "ui/ui_base.h"
#include "ui_elements/button.h"

/**
 * @struct ui_supplies
 * @brief Supplies screen UI components
 *
 * Contains all interactive elements for the supplies management.
 */
struct ui_supplies {
    struct ui_base base; ///< Base ui methods/functionality

    struct button butn_back; ///< Button to got back to main menu

    enum supplies_screen_flags flag; ///< Flags for the struct
};

/**
 * @brief Initializes supplies UI elements
 *
 * Sets up all elements with default positions and labels.
 *
 * @param ui Pointer to ui_supplies struct to initialize
 */
void ui_supplies_init(struct ui_supplies *ui);

#endif // UI_SUPPLIES_H

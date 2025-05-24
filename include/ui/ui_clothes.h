/**
 * @file ui_clothes.h
 * @brief Clothes Screen Management
 *
 * Handles the presentation and interaction of the application's
 * clothes management interface.
 */

#ifndef UI_CLOTHES_H
#define UI_CLOTHES_H

/**
 * @enum clothes_screen_flags
 * @brief State flags for the clothes screen
 *
 * Tracks various states and validation results for the clothes screen.
 */
enum clothes_screen_flags {
    FLAG_CLOTHES_OPERATION_DONE = 1 << 0, ///< Operation done
};

#include "ui/ui_base.h"
#include "ui_elements/button.h"

/**
 * @struct ui_clothes
 * @brief Clothes screen UI components
 *
 * Contains all interactive elements for the clothes management.
 */
struct ui_clothes {
    struct ui_base base; ///< Base ui methods/functionality

    struct button butn_back; ///< Button to got back to main menu

    enum clothes_screen_flags flag; ///< Flags for the struct
};

/**
 * @brief Initializes clothes UI elements
 *
 * Sets up all elements with default positions and labels.
 *
 * @param ui Pointer to ui_clothes struct to initialize
 */
void ui_clothes_init(struct ui_clothes *ui);

#endif // UI_CLOTHES_H

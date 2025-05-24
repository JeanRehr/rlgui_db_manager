/**
 * @file ui_medication.h
 * @brief Medication Screen Management
 *
 * Handles the presentation and interaction of the application's
 * medication management interface.
 */

#ifndef UI_MEDICATION_H
#define UI_MEDICATION_H

/**
 * @enum medication_screen_flags
 * @brief State flags for the medication screen
 *
 * Tracks various states and validation results for the medication screen.
 */
enum medication_screen_flags {
    FLAG_MEDICATION_OPERATION_DONE = 1 << 0, ///< Operation done
};

#include "ui/ui_base.h"
#include "ui_elements/button.h"

/**
 * @struct ui_medication
 * @brief Medication screen UI components
 *
 * Contains all interactive elements for the medication management.
 */
struct ui_medication {
    struct ui_base base; ///< Base ui methods/functionality

    struct button butn_back; ///< Button to got back to main menu

    enum medication_screen_flags flag; ///< Flags for the struct
};

/**
 * @brief Initializes medication UI elements
 *
 * Sets up all elements with default positions and labels.
 *
 * @param ui Pointer to ui_medication struct to initialize
 */
void ui_medication_init(struct ui_medication *ui);

#endif // UI_MEDICATION_H

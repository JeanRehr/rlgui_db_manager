/**
 * @file ui_resident.h
 * @brief Resident Registration Screen Management
 *
 * Manages all UI elements and state for the resident registration screen,
 * including form input, database operations, and user feedback.
 */

#ifndef UI_RESIDENT_H
#define UI_RESIDENT_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "resident.h"
#include "ui_elements/button.h"
#include "ui_elements/checkbox.h"
#include "ui_elements/dropdownbox.h"
#include "ui_elements/intbox.h"
#include "ui_elements/textbox.h"
#include "ui_elements/textboxint.h"

/**
 * @enum resident_screen_flags
 * @brief State flags for resident screen operations
 *
 * Bitmask flags that track various states and validation results
 * for the resident registration screen.
 */
enum resident_screen_flags {
    FLAG_RESIDENT_OPERATION_DONE = 1 << 0, ///< Submission completed successfully
    FLAG_CONFIRM_RESIDENT_DELETE = 1 << 1, ///< Pending delete confirmation
    FLAG_CPF_EXISTS = 1 << 2,              ///< CPF already exists in database
    FLAG_CPF_NOT_FOUND = 1 << 3,           ///< CPF not found in database
    FLAG_INPUT_CPF_EMPTY = 1 << 4,         ///< CPF input field is empty
    FLAG_CPF_NOT_VALID = 1 << 5,           ///< CPF input is invalid (not 11 digits)
    FLAG_SHOW_HEALTH = 1 << 6,             ///< Show full health status popup
    FLAG_SHOW_NEEDS = 1 << 7               ///< Show full needs description popup
};

/**
 * @struct ui_resident
 * @brief Complete resident registration screen state
 *
 * Contains all UI elements and state information needed to manage
 * the resident registration screen.
 */
struct ui_resident {
    struct textbox tb_name;                ///< Name input field
    struct textboxint tb_cpf;              ///< CPF input field (numeric with formatting)
    struct intbox ib_age;                  ///< Age input field
    struct textbox tb_health_status;       ///< Health status description
    struct textbox tb_needs;               ///< Special needs description
    struct checkbox cb_medical_assistance; ///< Medical assistance toggle
    struct dropdownbox ddb_gender;         ///< Gender selection dropdown

    struct button butn_back;         ///< Return to previous screen
    struct button butn_submit;       ///< Submit form data
    struct button butn_retrieve;     ///< Retrieve resident data
    struct button butn_delete;       ///< Delete resident record
    struct button butn_retrieve_all; ///< Show all residents

    Rectangle panel_bounds;             ///< Information display panel bounds
    struct resident resident_retrieved; ///< Currently displayed resident data
    enum resident_screen_flags flag;    ///< Current screen state flags
};

/**
 * @brief Initializes the resident registration screen
 *
 * Sets up all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_resident struct to initialize
 */
void ui_resident_init(struct ui_resident *ui);

/**
 * @brief Draws and updates the resident registration screen
 *
 * Handles rendering and interaction for all screen elements.
 *
 * @param ui Pointer to initialized ui_resident struct
 * @param state Pointer to application state (may be modified)
 * @param error Pointer to error code (may be modified)
 * @param resident_db Pointer to resident database connection
 */
void ui_resident_draw(struct ui_resident *ui, enum app_state *state, enum error_code *error, database *resident_db);

/**
 * @brief Updates element positions for window resizing
 *
 * Adjusts UI element positions based on current window dimensions.
 * Should be called when the window is resized.
 *
 * @param ui Pointer to ui_resident struct to update
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_resident_updt_pos(struct ui_resident *ui);

#endif // UI_RESIDENT_H
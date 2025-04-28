/**
 * @file ui_food.h
 * @brief Food Inventory Management Screen
 *
 * Provides interface for managing food batch records including:
 * - Adding new food batches
 * - Updating existing entries
 * - Viewing inventory details
 */

#ifndef UI_FOOD_H
#define UI_FOOD_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "foodbatch.h"
#include "ui_elements/button.h"
#include "ui_elements/checkbox.h"
#include "ui_elements/floatbox.h"
#include "ui_elements/intbox.h"
#include "ui_elements/textbox.h"

/**
 * @enum food_screen_flags
 * @brief State flags for food management operations
 *
 * Tracks various states and validation results for food batch management.
 */
enum food_screen_flags {
    FLAG_FOOD_OPERATION_DONE = 1 << 0, ///< Database operation completed
    FLAG_CONFIRM_FOOD_DELETE = 1 << 1, ///< Pending delete confirmation
    FLAG_BATCHID_EXISTS = 1 << 2,      ///< Batch ID already in database
    FLAG_INVALID_FOOD_DATE = 1 << 3,   ///< Invalid expiration date entered
    FLAG_BATCHID_NOT_FOUND = 1 << 4    ///< Specified batch ID not found
};

/**
 * @struct ui_food
 * @brief Complete food management screen state
 *
 * Contains all UI elements and state information needed to manage
 * food inventory records.
 */
struct ui_food {
    struct intbox ib_batch_id;        ///< Unique batch identifier input
    struct textbox tb_name;           ///< Food name description
    struct intbox ib_quantity;        ///< Current stock quantity
    struct checkbox cb_is_perishable; ///< Perishable status toggle

    Rectangle expirationDateText; ///< Expiration date label bounds
    struct intbox ib_year;        ///< Expiration year input
    struct intbox ib_month;       ///< Expiration month input
    struct intbox ib_day;         ///< Expiration day input

    struct floatbox fb_daily_consumption_rate; ///< Consumption rate input

    struct button butn_back;         ///< Navigation back button
    struct button butn_submit;       ///< Form submission button
    struct button butn_retrieve;     ///< Record retrieval button
    struct button butn_delete;       ///< Record deletion button
    struct button butn_retrieve_all; ///< Full inventory view button

    Rectangle panel_bounds;               ///< Information display panel
    struct foodbatch foodbatch_retrieved; ///< Currently displayed record
    enum food_screen_flags flag;          ///< Current operation flags
};

/**
 * @brief Initializes food management screen
 *
 * Sets up all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_food struct to initialize
 */
void ui_food_init(struct ui_food *ui);

/**
 * @brief Draws and manages food management screen
 *
 * Handles rendering and interaction for all screen elements.
 *
 * @param ui Pointer to initialized ui_food struct
 * @param state Pointer to application state (may be modified)
 * @param error Pointer to error tracking variable
 * @param foodbatch_db Pointer to food batch database connection
 */
void ui_food_draw(struct ui_food *ui, enum app_state *state, enum error_code *error, database *foodbatch_db);

/**
 * @brief Updates screen element positions
 *
 * Adjusts UI elements based on current window dimensions.
 *
 * @param ui Pointer to ui_food struct to update
 *
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_food_updt_pos(struct ui_food *ui);

#endif // UI_FOOD_H
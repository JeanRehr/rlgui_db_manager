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

#include "entities/foodbatch.h"
#include "ui/components/button.h"
#include "ui/components/checkbox.h"
#include "ui/components/floatbox.h"
#include "ui/components/intbox.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

/**
 * @enum food_screen_flags
 * @brief State flags for food management operations
 *
 * Tracks various states and validation results for food batch management.
 */
enum food_screen_flags {
    FLAG_FOOD_OPERATION_DONE = 1 << 0,       ///< Database operation completed
    FLAG_FOOD_GENERIC_ERROR = 1 << 1,        ///< Generic error
    FLAG_FOOD_CONFIRM_DELETE = 1 << 2,       ///< Pending delete confirmation
    FLAG_FOOD_BATCHID_EXISTS = 1 << 3,       ///< Batch ID already in database
    FLAG_FOOD_INVALID_EXP_DATE = 1 << 4,     ///< Invalid expiration date entered
    FLAG_FOOD_INVALID_ARRIVAL_DATE = 1 << 5, ///< Invalid arrival date entered
    FLAG_FOOD_BATCHID_NOT_FOUND = 1 << 6,    ///< Specified batch ID not found
    FLAG_FOOD_NAME_EMPTY = 1 << 7,           ///< Name empty
    FLAG_FOOD_QUANTITY_EMPTY = 1 << 8,       ///< Quantity empty
    FLAG_FOOD_UNIT_EMPTY = 1 << 9,           ///< Unit empty
};

/**
 * @struct ui_food
 * @brief Complete food management screen state
 *
 * Contains all UI elements and state information needed to manage
 * food inventory records.
 */
struct ui_food {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_name;           ///< Name of the food e.g. "Banana"
    struct floatbox fb_quantity;      ///< For both weight and count
    struct textbox tb_unit;           ///< e.g. "L", "KG", "Piece", "cans", "lbs"
    struct checkbox cb_is_perishable; ///< Perishable status toggle
    struct intbox ib_batch_id;        ///< Unique batch identifier, used for getting and deleting

    Rectangle arrival_date_text;    ///< Arrival date label bounds ISO format 2000-12-21
    struct intbox ib_arrival_year;  ///< Arrival year input
    struct intbox ib_arrival_month; ///< Arrival month input
    struct intbox ib_arrival_day;   ///< Arrival day input

    Rectangle expiration_date_text; ///< Expiration date label bounds Nullable, ISO format 2000-12-21
    struct intbox ib_exp_year;      ///< Expiration year input
    struct intbox ib_exp_month;     ///< Expiration month input
    struct intbox ib_exp_day;       ///< Expiration day input

    struct button butn_back;         ///< Navigation back button
    struct button butn_submit;       ///< Form submission button
    struct button butn_update;       ///< Update food button
    struct button butn_retrieve;     ///< Record retrieval button
    struct button butn_delete;       ///< Record deletion button
    struct button butn_retrieve_all; ///< Full inventory view button

    Rectangle panel_bounds;               ///< Information display panel
    struct foodbatch foodbatch_retrieved; ///< Currently displayed record

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the resident's database
    char *str_table_content;          ///< The content of the resident's database (MUST BE FREED IF ALLOCATED)

    enum food_screen_flags flag; ///< Current operation flags
};

/**
 * @brief Initializes food management screen
 *
 * Sets up base interface overrides and all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_food struct to initialize
 */
void ui_food_init(struct ui_food *ui);

#endif // UI_FOOD_H

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

#include "ui/components/button.h"
#include "ui/components/intbox.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

enum supplies_screen_flags {
    FLAG_SUPPLIES_OPERATION_DONE = 1 << 0,         ///< Operation done
    FLAG_SUPPLIES_GENERIC_ERROR = 1 << 1,          ///< Generic error
    FLAG_SUPPLIES_NOTFOUND = 1 << 2,               ///< Supplies entry not found
    FLAG_SUPPLIES_STOCK_BELOW_ZERO = 1 << 3,       ///< Operation will make stock go below 0
    FLAG_SUPPLIES_CONFIRM_REMOVAL = 1 << 4,        ///< Confirm removal of quantity
    FLAG_SUPPLIES_CONFIRM_DELETION = 1 << 5,       ///< Confirm deletion of entry
    FLAG_SUPPLIES_CONFIRM_REMOVAL_BY_ID = 1 << 6,  ///< Confirm removal of quantity by ID
    FLAG_SUPPLIES_CONFIRM_DELETION_BY_ID = 1 << 7, ///< Confirm deletion of entry by ID
    FLAG_SUPPLIES_NAME_EMPTY = 1 << 8,             ///< Name textbox empty
    FLAG_SUPPLIES_CATEGORY_EMPTY = 1 << 9,         ///< Category textbox empty
    FLAG_SUPPLIES_SIZE_EMPTY = 1 << 10,            ///< Size textbox empty
};

/**
 * @struct ui_supplies
 * @brief Supplies screen UI components
 *
 * Contains all interactive elements for the supplies management.
 */
struct ui_supplies {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_name;     ///< e.g. "diaper", "tampon"
    struct textbox tb_category; ///< e.g. "adult", "small", "xxl"
    struct textbox tb_size;     // e.g. "hygiene", "cleaning", "personal care"
    struct textbox tb_unit;     ///< e.g. "piece", "pack", "box"
    struct intbox ib_stock;     ///< Current count in inventory
    struct textbox tb_notes;    ///< General notes if needed

    struct intbox ib_supply_id; ///< ID of the supply, used for removal/deletion

    struct button butn_back;               ///< Button to got back to main menu
    struct button butn_insert;             ///< Button to insert/update into the database
    struct button butn_remove;             ///< Button to remove a med by quantity from the database
    struct button butn_delete_entry;       ///< Button to delete the entry from the database
    struct button butn_remove_by_id;       ///< Button to remove by Supplies entry ID
    struct button butn_delete_entry_by_id; ///< Button to delete entry by Supplies entry ID
    struct button butn_view_all;           ///< Button to get a database view

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the medication database
    char *str_table_content;          ///< The content of the medication database (MUST BE FREED IF ALLOCATED)

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

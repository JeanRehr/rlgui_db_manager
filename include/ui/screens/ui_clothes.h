/**
 * @file ui_clothes.h
 * @brief Clothes Screen Management
 *
 * Handles the presentation and interaction of the application's
 * clothes management interface.
 */

#ifndef UI_CLOTHES_H
#define UI_CLOTHES_H

#include "entities/clothing.h"
#include "ui/components/button.h"
#include "ui/components/dropdownbox.h"
#include "ui/components/intbox.h"
#include "ui/components/listview.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

/**
 * @enum clothes_screen_flags
 * @brief State flags for the clothes screen
 *
 * Tracks various states and validation results for the clothes screen.
 */
enum clothes_screen_flags {
    FLAG_CLOTHES_OPERATION_DONE = 1 << 0,         ///< Operation done
    FLAG_CLOTHES_GENERIC_ERROR = 1 << 1,          ///< Generic database operation error
    FLAG_CLOTHES_NOTFOUND = 1 << 2,               ///< Clothing entry not found
    FLAG_CLOTHES_STOCK_BELOW_ZERO = 1 << 3,       ///< Operation will make stock go below 0
    FLAG_CLOTHES_CONFIRM_REMOVAL = 1 << 4,        ///< Confirm removal of quantity
    FLAG_CLOTHES_CONFIRM_DELETION = 1 << 5,       ///< Confirm deletion of entry
    FLAG_CLOTHES_CONFIRM_REMOVAL_BY_ID = 1 << 6,  ///< Confirm removal of quantity by ID
    FLAG_CLOTHES_CONFIRM_DELETION_BY_ID = 1 << 7, ///< Confirm deletion of entry by ID
};

/**
 * @struct ui_clothes
 * @brief Clothes screen UI components
 *
 * Contains all interactive elements for the clothes management.
 *
 * @note These type options are meant to be enums in clothing.h
 *
 * @warning These type options MUST follow the same naming and ORDER as in the corresponding enum on clothing.h enum
 */
struct ui_clothes {
    struct ui_base base; ///< Base ui methods/functionality

    struct listview lv_type;          ///< Types option
    struct listview lv_size;          ///< Sizes option
    struct dropdownbox ddb_gender;    ///< Gender options
    struct listview lv_color;         ///< Color options
    struct dropdownbox ddb_condition; ///< Condition options

    struct intbox ib_quantity; ///< Quantity being inserted/updated

    struct textbox tb_notes; ///< General notes about the item being inserted

    struct intbox ib_clothes_id; ///< ID of the clothes, used for removal/deletion

    struct button butn_back;               ///< Button to go back to main menu
    struct button butn_insert;             ///< Button to insert/update into the database
    struct button butn_remove;             ///< Button to remove a clothing by quantity from the database
    struct button butn_delete_entry;       ///< Button to delete the entry from the database
    struct button butn_remove_by_id;       ///< Button to remove by Clothes ID
    struct button butn_delete_entry_by_id; ///< Button to delete entry by Clothes ID
    struct button butn_view_all;           ///< Button to get a database view

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the clothes database
    char *str_table_content;          ///< The content of the clothes database (MUST BE FREED IF ALLOCATED)

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

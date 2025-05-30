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
    FLAG_MEDICATION_OPERATION_DONE = 1 << 0,         ///< Operation done
    FLAG_MEDICATION_GENERIC_ERROR = 1 << 1,          ///< Generic database operation error
    FLAG_MEDICATION_NOTFOUND = 1 << 2,               ///< Medication entry not found
    FLAG_MEDICATION_STOCK_BELOW_ZERO = 1 << 3,       ///< Operation will make stock go below 0
    FLAG_MEDICATION_CONFIRM_REMOVAL = 1 << 4,        ///< Confirm removal of quantity
    FLAG_MEDICATION_CONFIRM_DELETION = 1 << 5,       ///< Confirm deletion of entry
    FLAG_MEDICATION_CONFIRM_REMOVAL_BY_ID = 1 << 6,  ///< Confirm removal of quantity by ID
    FLAG_MEDICATION_CONFIRM_DELETION_BY_ID = 1 << 7, ///< Confirm deletion of entry by ID
    FLAG_MEDICATION_INVALID_DATE = 1 << 8,           ///< Invalid date inserted
    FLAG_MEDICATION_NAME_EMPTY = 1 << 9,             ///< Name textbox empty
    FLAG_MEDICATION_FORM_EMPTY = 1 << 10,            ///< Form textbox empty
    FLAG_MEDICATION_STRENGTH_EMPTY = 1 << 11,        ///< Strength textbox empty
};

#include "ui/components/button.h"
#include "ui/components/intbox.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

/**
 * @struct ui_medication
 * 
 * @brief Medication screen UI components
 * 
 */
struct ui_medication {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_name;         ///< e.g. "Paracetamol 500g"
    struct textbox tb_generic_name; ///< e.g. "Paracetamol"
    struct textbox tb_form;         ///< e.g. "Tablet", "Syrup", "Injection"
    struct textbox tb_strength;     ///< e.g. "500mg", "5mg/ml"
    struct textbox tb_unit;         ///< e.g. "Tablet", "ml", "vial"
    struct intbox ib_stock;         ///< Current count in inventory

    Rectangle expirationDateText; ///< Expiration date label bounds
    struct intbox ib_year;        ///< Expiration year input
    struct intbox ib_month;       ///< Expiration month input
    struct intbox ib_day;         ///< Expiration day input

    struct textbox tb_notes; ///< General notes if needed

    struct intbox ib_medication_id; ///< ID of the medication, used for removal/deletion

    struct button butn_back;               ///< Button to got back to main menu
    struct button butn_insert;             ///< Button to insert/update into the database
    struct button butn_remove;             ///< Button to remove a med by quantity from the database
    struct button butn_delete_entry;       ///< Button to delete the entry from the database
    struct button butn_remove_by_id;       ///< Button to remove by Medications entry ID
    struct button butn_delete_entry_by_id; ///< Button to delete entry by Medications entry ID
    struct button butn_view_all;           ///< Button to get a database view

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the medication database
    char *str_table_content;          ///< The content of the medication database (MUST BE FREED IF ALLOCATED)

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

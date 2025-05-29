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

#include "ui/components/button.h"
#include "ui/components/intbox.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

/**
 * @struct ui_medication
 * @brief Medication screen UI components
 *
 * Contains all interactive elements for the medication management.
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"               // e.g. "Paracetamol 500g"
        "GenericName TEXT,"                 // e.g. "Paracetamol"
        "Form TEXT,"                        // e.g. "Tablet", "Syrup", "Injection"
        "Strength TEXT,"                    // e.g. "500mg", "5mg/ml"
        "Unit TEXT,"                        // e.g. "Tablet", "ml", "vial"
        "Stock INTEGER NOT NULL DEFAULT 0," // Current count in inventory
        "ExpirationDate TEXT,"              // Soonest expiration date
        "Notes TEXT,"                       // General notes if needed
        "UNIQUE(Name, Form, Strength));";   // Prevents accidental duplicate entries of the same medication in the same
                                            // dosage and form e.g. multiple "Paracetamol Tablet 500mg"
 */
struct ui_medication {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_name;         ///< e.g. "Paracetamol 500g"
    struct textbox tb_generic_name; ///< e.g. "Paracetamol"
    struct textbox tb_form;         ///< e.g. "Tablet", "Syrup", "Injection"
    struct textbox tb_strength;      ///< e.g. "500mg", "5mg/ml"
    struct textbox tb_unit;         ///< e.g. "Tablet", "ml", "vial"
    struct intbox ib_stock;         ///< Current count in inventory

    Rectangle expirationDateText; ///< Expiration date label bounds
    struct intbox ib_year;        ///< Expiration year input
    struct intbox ib_month;       ///< Expiration month input
    struct intbox ib_day;         ///< Expiration day input

    struct textbox tb_notes; ///< General notes if needed

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

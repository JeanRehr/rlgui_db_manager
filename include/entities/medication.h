/**
 * @file medication.h
 * @brief Medication definition for use in database operations/code
 */
#ifndef MEDICATION_H
#define MEDICATION_H

#include <stdbool.h>

#include "global/CONSTANTS.h"

/**
 * @struct medication
 *
 * @brief Represents a pieace of medication record in the database
 *
 */
struct medication {
    char *name;                     ///< e.g. "Paracetamol 500g"
    char *generic_name;             ///< e.g. "Paracetamol"
    char *form;                     ///< e.g. "Tablet", "Syrup", "Injection"
    char *strength;                 ///< e.g. "500mg", "5mg/ml"
    char *unit;                     ///< e.g. "Tablet", "ml", "vial"
    int stock;                      ///< // Current count in inventory
    char expiration_date[DATE_LEN]; ///< Soonest expiration date
    char notes[MAX_INPUT];          ///< General notes if needed
};

#endif // MEDICATION_H

/**
 * @file medication.h
 * @brief Medication definition for use in database operations/code
 */
#ifndef MEDICATION_H
#define MEDICATION_H

#include "global/CONSTANTS.h"

/** 
 * @struct medication
 *
 * @brief Represents a piece of medication record in the database
 *
 */
struct medication {
    char name[MAX_INPUT];                     ///< e.g. "Paracetamol 500g"
    char generic_name[MAX_INPUT];             ///< e.g. "Paracetamol"
    char form[MAX_INPUT];                     ///< e.g. "Tablet", "Syrup", "Injection"
    char strength[MAX_INPUT];                 ///< e.g. "500mg", "5mg/ml"
    char unit[MAX_INPUT];                     ///< e.g. "Tablet", "ml", "vial"
    int stock;                      ///< // Current count in inventory
    char expiration_date[DATE_LEN]; ///< Soonest expiration date
    char notes[MAX_INPUT];          ///< General notes if needed
};

#endif // MEDICATION_H

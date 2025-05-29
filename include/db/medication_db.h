/**
 * @file medication_db.h
 * @brief Medications Database Operations
 *
 * This header defines operations for managing medication records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of medication information.
 */

#ifndef MEDICATION_DB_H
#define MEDICATION_DB_H

#include "db_manager.h"

#include "entities/medication.h"

/**
 * @brief Creates the Medications table in the database
 *
 * Creates a new Medications table if it doesn't already exist. The table includes fields for
 * ID, Name, GenericaName, Form, Strength, Unit, Stock, Notes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int medication_db_create_table(database *db);

/**
 * @brief Inserts/updates a new medication record into the database
 *
 * Adds a new record to the Medications table with the provided parameters.
 * If the record already exists, then it updates the quantity.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "Paracetamol 500g"
 * @param[in] generic_name e.g. "Paracetamol"
 * @param[in] form e.g. Tablet", "Syrup", "Injection"
 * @param[in] strength "500mg", "5mg/ml"
 * @param[in] unit "Tablet", "ml", "vial"
 * @param[in] stock Quantity being inserted, or to update
 * @param[in] expiration_date Soonest expiration date
 * @param[in] notes General notes if needed
 *
 * @note Param notes and expiration date can be null, then it will not be updated
 *
 * @return SQLITE_OK on success, SQLITE_CONSTRAINT on quantity less than 0, SQLite error code on failure
 *
 */
int medication_db_upsert(
    database *db,
    const char *name,
    const char *generic_name,
    const char *form,
    const char *strength,
    const char *unit,
    const int stock,
    const char *expiration_date,
    const char *notes
);

/**
 * @brief Checks if a Medications records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "Paracetamol 500g"
 * @param[in] form e.g. Tablet", "Syrup", "Injection"
 * @param[in] strength "500mg", "5mg/ml"
 *
 * @return true if the record exists, false otherwise
 */
bool medication_db_check_exists(database *db, const char *name, const char *form, const char *strength);

/**
 * @brief Checks if a Medications records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the medication to check
 *
 * @return true if the record exists, false otherwise
 */
bool medication_db_check_exists_by_id(database *db, const int id);

/**
 * @brief Removes a medication from the Medications table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "Paracetamol 500g"
 * @param[in] form e.g. Tablet", "Syrup", "Injection"
 * @param[in] strength "500mg", "5mg/ml"
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the medication doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 */
int medication_db_remove(
    database *db,
    const char *name,
    const char *form,
    const char *strength,
    const int quantity_to_remove
);

/**
 * @brief Removes a medication from the Medications table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the medication to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the medication doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 */
int medication_db_remove_by_id(database *db, const int id, const int quantity_to_remove);

/**
 * @brief Deletes a medication record by its UNIQUE data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "Paracetamol 500g"
 * @param[in] form e.g. Tablet", "Syrup", "Injection"
 * @param[in] strength "500mg", "5mg/ml"
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int medication_db_delete_entry(database *db, const char *name, const char *form, const char *strength);

/**
 * @brief Deletes a medication record by its ID data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the medication to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int medication_db_delete_entry_by_id(database *db, const int id);

/**
 * 
 * @brief Retrieves a medication record
 *
 * Fetches the complete record for the medication table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "Paracetamol 500g"
 * @param[in] form e.g. Tablet", "Syrup", "Injection"
 * @param[in] strength "500mg", "5mg/ml"
 * @param[out] medication struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int medication_db_get(
    database *db,
    const char *name,
    const char *form,
    const char *strength,
    struct medication *medication
);

/**
 * @brief Gets the count of registered medication in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of medication on success, -1 on failure
 */
int medication_db_get_count(database *db);

/**
 * @brief Retrieves and displays all medication records
 *
 * Fetches all records from the Medications table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int medication_db_get_all(database *db);

#endif // MEDICATION_DB_H

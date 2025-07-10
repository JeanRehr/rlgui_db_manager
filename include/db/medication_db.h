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

#include <stddef.h>

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
 * @param[in] id ID of the medication to remove the quantity\
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
 * 
 * @brief Retrieves a medication record by ID
 *
 * Fetches the complete record for the medication table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the entry to fetch the medication
 * @param[out] medication struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int medication_db_get_by_id(
    database *db,
    const int id,
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
 * @brief Writes all medication records as a formatted string into provided buffer
 *
 * Executes a database query and formats all medication records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * @note Header will always needs 540 bytes and each row + separator will need at max 1765
 *       (considering max input is 256) with the current table and format
 *       String format:
 * +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Name                     | GenericName              | Form             | Strength         | Unit                     | Stock | ExpirationDate | Notes                    |
 * +-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+
 * | 1   | Paracetamol              | Paracetamol 500mg        | Tablet           | 500mg            | Tablet                   | 6     | 2025-12-20     |                          |
 * +-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+
 * 
 * @warning Returns -1 if database is not initialized, on query failure, or if buffer is too small
 * @warning Buffer will be null-terminated if there's space, even on truncation
 *
 * Memory Management:
 * 
 * - Caller provides buffer and manages its memory
 * 
 * - Function never allocates memory
 *
 * Error Handling:
 * 
 * - Checks database connection state
 * 
 * - Validates SQL preparation
 * 
 * - Reports SQL execution errors
 * 
 * - Handles buffer overflow
 * 
 */
int medication_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/** 
 * @brief Retrieves all medication records as a formatted string
 *
 * Executes a database query and formats all medication records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Name                     | GenericName              | Form             | Strength         | Unit                     | Stock | ExpirationDate | Notes                    |
 * +-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+
 * | 1   | Paracetamol              | Paracetamol 500mg        | Tablet           | 500mg            | Tablet                   | 6     | 2025-12-20     |                          |
 * +-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+
 * 
 * @note Header will always needs 540 bytes and each row + separator will need at max 1765 with the current table and format
 *
 * @warning The caller is responsible for freeing the returned string with free()
 * @warning Returns NULL if database is not initialized or on query failure
 * @warning This is not safe as it does not adhere to the memory encapsulation principle
 *
 * Memory Management:
 * - Allocates initial 4KB buffer
 * - Automatically grows buffer as needed
 * - Returns NULL on allocation failures
 *
 * Error Handling:
 * 
 * - Checks database connection state
 * 
 * - Validates SQL preparation
 * 
 * - Handles memory allocation failures
 * 
 * - Reports SQL execution errors
 *
 * @warning THIS SHOULD NOT BE USED
 *
 */
char *medication_db_get_all_format_old(database *db);

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

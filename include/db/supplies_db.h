/**
 * @file supplies_db.h
 * @brief Supplies Database Operations
 *
 * This header defines operations for managing supplies records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of supplies information.
 */

#ifndef SUPPLIES_DB_H
#define SUPPLIES_DB_H

#include "db_manager.h"

#include <stddef.h>

#include "entities/supply.h"

/**
 * @brief Creates the Supplies table in the database
 *
 * Creates a new Supplies table if it doesn't already exist. The table includes fields for
 * ID, Name, Category, Size, Unit, Quantity, Notes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int supplies_db_create_table(database *db);

/**
 * @brief Inserts/updates a new supplies record into the database
 *
 * Adds a new record to the Supplies table with the provided parameters.
 * If the record already exists, then it updates the quantity.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "diaper", "tampon"
 * @param[in] category e.g. "hygiene", "cleaning", "personal care"
 * @param[in] size "adult", "small", "XXL"
 * @param[in] unit "piece", "pack" "box"
 * @param[in] stock Quantity being inserted, or to update
 * @param[in] notes General notes if needed
 *
 * @note Param notes can be null, then it will not be updated
 *
 * @return SQLITE_OK on success, SQLITE_CONSTRAINT on quantity less than 0, SQLite error code on failure
 *
 */
int supplies_db_upsert(
    database *db,
    const char *name,
    const char *category,
    const char *size,
    const char *unit,
    const int stock,
    const char *notes
);

/**
 * @brief Checks if a supply records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "diaper", "tampon"
 * @param[in] category e.g. "hygiene", "cleaning", "personal care"
 * @param[in] size "adult", "small", "XXL"
 *
 * @return true if the record exists, false otherwise
 */
bool supplies_db_check_exists(database *db, const char *name, const char *category, const char *size);

/**
 * @brief Checks if a Supplies records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the supplies to check
 *
 * @return true if the record exists, false otherwise
 */
bool supplies_db_check_exists_by_id(database *db, const int id);

/**
 * @brief Removes a supply from the Supplies table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "diaper", "tampon"
 * @param[in] category e.g. "hygiene", "cleaning", "personal care"
 * @param[in] size "adult", "small", "XXL"
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the supply doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 */
int supplies_db_remove(
    database *db,
    const char *name,
    const char *category,
    const char *unit,
    const int quantity_to_remove
);

/**
 * @brief Removes a supply from the Supplies table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the supply to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the supply doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 */
int supplies_db_remove_by_id(database *db, const int id, const int quantity_to_remove);

/**
 * @brief Deletes a supply record by its UNIQUE data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "diaper", "tampon"
 * @param[in] category e.g. "hygiene", "cleaning", "personal care"
 * @param[in] size "adult", "small", "XXL"
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int supplies_db_delete_entry(database *db, const char *name, const char *category, const char *size);

/**
 * @brief Deletes a supply record by its ID data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the supply to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int supplies_db_delete_entry_by_id(database *db, const int id);

/**
 * 
 * @brief Retrieves a supply record
 *
 * Fetches the complete record for the Supplies table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] name e.g. "diaper", "tampon"
 * @param[in] category e.g. "hygiene", "cleaning", "personal care"
 * @param[in] size "adult", "small", "XXL"
 * @param[out] out_supply struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int supplies_db_get(database *db, const char *name, const char *category, const char *size, struct supply *out_supply);

/**
 * 
 * @brief Retrieves a supply record by ID
 *
 * Fetches the complete record for the Supplies table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the entry to fetch
 * @param[out] out_supply struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int supplies_db_get_by_id(database *db, const int id, struct supply *out_supply);

/**
 * @brief Gets the count of registered supplies in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of supplies on success, -1 on failure
 */
int supplies_db_get_count(database *db);

/**
 * @brief Writes all supplies records as a formatted string into provided buffer
 *
 * Executes a database query and formats all supplies records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * 
 * @note Header will always needs 405 (406 (+\0) if no other row) bytes and each row + separator
 *       (Considering max input is 256) will need at max 1450 with the current table and format
 *       String format:
 * +------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Name                     | Category                 | Size             | Unit             | Stock | Notes                    |
 * +-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+
 * | 1   | Diaper                   | Personal Care            | M                | Pack             | 5     | Pampers                  |
 * +-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+
 *
 * @warning Returns -1 if database is not initialized, on query failure, or if buffer is too small
 * @warning Buffer will be null-terminated if there's space, even on truncation
 *
 * Memory Management:
 * - Caller provides buffer and manages its memory
 * - Function never allocates memory
 *
 * Error Handling:
 * - Checks database connection state
 * - Validates SQL preparation
 * - Reports SQL execution errors
 * - Handles buffer overflow
 */
int supplies_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @deprecated
 * 
 * @brief Retrieves all supplies records as a formatted string
 *
 * Executes a database query and formats all supplies records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Name                     | Category                 | Size             | Unit             | Stock | Notes                    |
 * +-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+
 * | 1   | Diaper                   | Personal Care            | M                | Pack             | 5     | Pampers                  |
 * +-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+
 *
 * @note Header will always needs 601 bytes and each row + separator (201) will need at max 1040 with the current table and format
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
 * - Checks database connection state
 * - Validates SQL preparation
 * - Handles memory allocation failures
 * - Reports SQL execution errors
 *
 */
char *supplies_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all Supplies records
 *
 * Fetches all records from the Supplies table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int supplies_db_get_all(database *db);

#endif // SUPPLIES_DB_H

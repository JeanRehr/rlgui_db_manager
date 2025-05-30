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
 * @brief Gets the count of registered supplies in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of supplies on success, -1 on failure
 */
int supplies_db_get_count(database *db);

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

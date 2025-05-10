/**
 * @file foodbatch_db.h
 * @brief Food Batch Database Operations
 *
 * This header defines operations for managing food batches in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of food batch records.
 * All functions require a valid database connection from db_manager.
 */

#ifndef FOOBATCH_DB_H
#define FOOBATCH_DB_H

#include "db_manager.h"
#include "foodbatch.h" // Requires struct foodbatch definition

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Creates the FoodBatch table in the database
 *
 * Creates a new FoodBatch table if it doesn't already exist. The table includes fields for
 * batch ID, name, quantity, perishable status, expiration date, and consumption rate.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection, table will not be created if already exists
 */
int foodbatch_db_create_table(database *db);

/**
 * @brief Inserts a new food batch record into the database
 *
 * Adds a new record to the FoodBatch table with the provided parameters.
 * All parameters are required and must be valid.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] batch_id Unique identifier for the new batch
 * @param[in] name Name/description of the food batch
 * @param[in] quantity Quantity of items in the batch
 * @param[in] isPerishable Whether the batch is perishable (true/false)
 * @param[in] expirationDate Expiration date string (YYYY-MM-DD format)
 * @param[in] dailyConsumptionRate Expected daily consumption rate
 * @return SQLITE_OK on success, SQLite error code on failure
 */
int foodbatch_db_insert(
    database *db,
    int batch_id,
    const char *name,
    int quantity,
    bool isPerishable,
    const char *expirationDate,
    float dailyConsumptionRate
);

/**
 * @brief Updates an existing food batch record
 *
 * Modifies the fields of an existing food batch record identified by batch_id.
 * Empty strings or negative values for numeric fields will preserve the existing values.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] batch_id ID of the batch to update
 * @param[in] name_input New name (empty string preserves current)
 * @param[in] quantity_input New quantity (<= 0 preserves current)
 * @param[in] is_perishable_input New perishable status (-1 preserves current)
 * @param[in] expiration_date_input New expiration date (empty string preserves current)
 * @param[in] daily_consumption_rate_input New consumption rate (< 0 preserves current)
 * @return SQLITE_OK on success, SQLite error code on failure
 */
int foodbatch_db_update(
    database *db,
    int batch_id,
    const char *name_input,
    int quantity_input,
    bool is_perishable_input,
    const char *expiration_date_input,
    float daily_consumption_rate_input
);

/**
 * @brief Deletes a food batch record by ID
 *
 * Removes the food batch record with the specified batch_id from the database.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] batch_id ID of the batch to delete
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if batch doesn't exist, or other SQLite error code
 */
int foodbatch_db_delete_by_id(database *db, int batch_id);

/**
 * @brief Retrieves a food batch record by ID
 *
 * Fetches the complete record for the food batch with the specified batch_id.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] batch_id ID of the batch to retrieve
 * @param[out] foodbatch Pointer to structure where the data will be stored
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if batch doesn't exist, or other SQLite error code
 */
int foodbatch_db_get_by_batchid(database *db, int batch_id, struct foodbatch *foodbatch);

/**
 * @brief Checks if a batch ID exists in the database
 *
 * Verifies whether a food batch with the specified ID exists in the database.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] batch_id ID to check for existence
 * @return true if the batch exists, false otherwise
 */
bool foodbatch_db_check_batchid_exists(database *db, int batch_id);

/**
 * @brief Gets the count of registered foodbatch in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of foodbatch on success, -1 on failure
 */
int foodbatch_db_get_count(database *db);

/**
 * @brief Writes all foodbatch records as a formatted string into provided buffer
 *
 * Executes a database query and formats all foodbatch records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * @note Header will always needs 307 bytes and each row + separator (Considering max input is 256)
 *       will need at max 455 with the current table and format
 *       String format:
 * +---------------------------------------------------------------------------------------------------+
 * | BatchId | Name                             | Quantity | Perishable | Expiration date | Daily Rate |
 * +---------+----------------------------------+----------+------------+-----------------+------------+
 * |       1 | Milk                             | 10       | True       | 2000-01-30      | 2.00       |
 * +---------+----------------------------------+----------+------------+-----------------+------------+
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
int foodbatch_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @brief Retrieves all foodbatch records as a formatted string
 *
 * Executes a database query and formats all foodbatch records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +---------------------------------------------------------------------------------------------------+
 * | BatchId | Name                             | Quantity | Perishable | Expiration date | Daily Rate |
 * +---------+----------------------------------+----------+------------+-----------------+------------+
 * |       1 | Milk                             | 10       | True       | 2000-01-30      | 2.00       |
 * +---------+----------------------------------+----------+------------+-----------------+------------+
 * 
 * @warning The caller is responsible for freeing the returned string with free()
 * @warning Returns NULL if database is not initialized or on query failure
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
 */
char *foodbatch_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all food batch records
 *
 * Fetches all records from the FoodBatch table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int foodbatch_db_get_all(database *db);

#endif // FOOBATCH_DB_H
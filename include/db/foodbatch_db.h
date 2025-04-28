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
 #include <stdbool.h>
 #include "foodbatch.h" // Requires struct foodbatch definition
 
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
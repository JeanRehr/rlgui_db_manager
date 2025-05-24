/**
 * @file clothes_db.h
 * @brief Clothes Database Operations
 *
 * This header defines operations for managing clothes records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of clothes information.
 */

#ifndef CLOTHES_DB_H
#define CLOTHES_DB_H

#include "db_manager.h"

/**
 * @brief Creates the Clothes table in the database
 *
 * Creates a new Clothes table if it doesn't already exist. The table includes fields for
 * ID, Type, Size, Gender, Color, Quantity, Condition, Notes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int clothes_db_create_table(database *db);

#endif // CLOTHES_DB_H
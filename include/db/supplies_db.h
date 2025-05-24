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

#endif // SUPPLIES_DB_H
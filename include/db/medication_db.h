/**
 * @file medication_db.h
 * @brief Medication Database Operations
 *
 * This header defines operations for managing medication records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of medication information.
 */

#ifndef MEDICATION_DB_H
#define MEDICATION_DB_H

#include "db_manager.h"

/**
 * @brief Creates the Medication table in the database
 *
 * Creates a new Medication table if it doesn't already exist. The table includes fields for
 * ID, Name, GenericaName, Form, Strength, Unit, Stock, Notes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int medication_db_create_table(database *db);

#endif // MEDICATION_DB_H
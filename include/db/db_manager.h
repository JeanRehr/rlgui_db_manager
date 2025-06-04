/**
 * @file db_manager.h
 * @brief SQLite3 Database Manager Interface
 *
 * This header defines a simple interface for managing SQLite3 database connections,
 * including initialization, deinitialization, and table creation.
 */

#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <stdbool.h>

#include <external/sqlite3/sqlite3.h>

#include "utils/logger.h"

/**
 * @struct database
 * @brief Represents a SQLite3 database connection.
 */
typedef struct database {
    sqlite3 *db;           ///< Internal SQLite3 database handle.
    struct logger *logger; ///< Logger to log database actions
} database;

/**
 * @brief Initializes a database connection.
 *
 * Opens an SQLite3 database file. If the file doesn't exist, it will be created.
 *
 * @param[out] db Pointer to the database structure to initialize.
 * @param[in] filename Path to the SQLite3 database file.
 * @param[in] logger Pointer to the app logger to call in database functions, can pass null.
 *
 * @return SQLITE_OK on success, SQLITE_ERROR on db or filename null, Sqlite error code another failure.
 *
 * @warning If this fails, `db->db` may be left in an invalid state.
 *          Every database function must check for logger null before calling a logger function
 *
 */
int db_init(database *db, const char *filename, struct logger *logger);

/**
 * @brief Initializes a database and creates a table via a callback.
 *
 * Combines `db_init()` with a user-provided table creation function. If either step fails,
 * the database is closed (if opened) and an error code is returned.
 *
 * @param[out] db Pointer to the database structure.
 * @param[in] filename Path to the database file.
 * @param[in] create_table Callback function to create tables.
 * @param[in] logger Pointer to the app logger to call in database functions, can pass null.
 * @return SQLITE_OK on success SQLITE_ERROR if db_init or create_table fails
 *         or if create_table callback is null
 *
 * @note On create_table failure, the database connection is automatically closed.
 * 
 * @warning Every database function must check for logger null before calling a logger function
 * 
 */
int db_init_with_tbl(database *db, const char *filename, int (*create_table)(database *), struct logger *logger);

/**
 * @brief Checks if the database connection is valid.
 *
 * @param[in] db Pointer to the database structure.
 *
 * @return true if db->db is non-NULL, false otherwise and if db is null.
 *
 * @note This is a simple NULL check and doesn't verify connection liveliness.
 *
 */
bool db_is_init(database *db);

/**
 * @brief Closes the database connection and resets the handle.
 *
 * Safely deinitializes the database. If db->db or db is NULL, this is a no-op.
 *
 * @param[in] db Pointer to the database structure.
 *
 * @warning After calling this, `db->db` will be NULL and must be reinitialized.
 *
 */
void db_deinit(database *db);

#endif // DB_MANAGER_H

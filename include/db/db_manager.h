/**
 * @file db_manager.h
 * @brief SQLite3 Database Manager Interface
 *
 * This header defines a simple interface for managing SQLite3 database connections,
 * including initialization, deinitialization, and table creation.
 */

#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <external/sqlite3/sqlite3.h>
#include <stdbool.h>

/**
 * @struct database
 * @brief Represents a SQLite3 database connection.
 */
typedef struct database {
    sqlite3 *db; ///< Internal SQLite3 database handle.
} database;

/**
 * @brief Initializes a database connection.
 *
 * Opens an SQLite3 database file. If the file doesn't exist, it will be created.
 * On failure, prints an error message to `stderr`.
 *
 * @param[out] db Pointer to the database structure to initialize.
 * @param[in] filename Path to the SQLite3 database file.
 * @return SQLITE_OK on success, SQLite error code on failure.
 * @warning If this fails, `db->db` may be left in an invalid state.
 */
int db_init(database *db, const char *filename);

/**
 * @brief Initializes a database and creates a table via a callback.
 *
 * Combines `db_init()` with a user-provided table creation function. If either step fails,
 * the database is closed (if opened) and an error code is returned.
 *
 * @param[out] db Pointer to the database structure.
 * @param[in] filename Path to the database file.
 * @param[in] create_table Callback function to create tables.
 * @return SQLITE_OK on success, or:
 *   - `ERROR_OPENING_DB` if `db_init()` fails.
 *   - `ERROR_CREATING_TABLE_DB` if `create_table()` fails.
 * @note On `create_table` failure, the database connection is automatically closed.
 */
int db_init_with_tbl(database *db, const char *filename, int (*create_table)(database *));

/**
 * @brief Checks if the database connection is valid.
 *
 * @param[in] db Pointer to the database structure.
 * @return `true` if `db->db` is non-NULL, `false` otherwise.
 * @note This is a simple NULL check and doesn't verify connection liveliness.
 */
bool db_is_init(database *db);

/**
 * @brief Closes the database connection and resets the handle.
 *
 * Safely deinitializes the database. If `db->db` is NULL, this is a no-op.
 *
 * @param[in] db Pointer to the database structure.
 * @warning After calling this, `db->db` will be NULL and must be reinitialized.
 */
void db_deinit(database *db);

#endif // DB_MANAGER_H
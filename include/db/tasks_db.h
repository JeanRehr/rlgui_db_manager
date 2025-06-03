/**
 * @file tasks_db.h
 * @brief Tasks Database Operations
 *
 * This header defines operations for managing the task list in an SQLite database.
 *
 */

#ifndef TASKS_DB_H
#define TASKS_DB_H

#include "db/db_manager.h"

#include <stddef.h>

#include "entities/task.h"

/**
 * @brief Creates the Tasks table in the database
 *
 * Creates a new Tasks table if it doesn't already exist, including fields for
 * authentication, administration, and timestamps.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * 
 * @warning Requires an initialized database connection
 */
int tasks_db_create_table(database *db);

/**
 * @brief Inserts/updates a new task into the database
 *
 * Adds a new record to the Tasks table with the provided parameters.
 * If the ID is filled (non-zero), then it tries to update.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID to update (leave zero when inserting)
 * @param[in] title Title of the task
 * @param[in] description Description
 * @param[in] due_date When it has to be completed
 * @param[in] priority Priority (0=low, 1=normal 2=high)
 * @param[in] status Status (0=pending, 1=in_progress, 2=done, 3=cancelled)
 * @param[in] assigned_to UserID the task will be assigned to (may be null)
 * @param[in] completed_at When the task was completed (null if not yet completed)
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND on an update where ID does not exists, 
 *         SQLite error code or other code on failure
 *
 */
int tasks_db_upsert(
    database *db,
    int id,
    const char *title,
    const char *description,
    const char *due_date,
    enum task_priority priority,
    enum task_status status,
    const char *assigned_to,
    const char *completed_at
);

/**
 * @brief Checks if a task record exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the task
 *
 * @return true if the record exists, false otherwise
 *
 */
bool tasks_db_check_exists(database *db, int id);

/**
 * @brief Delete all the records with the given status
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] status Status to be deleted
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND when no rows are modified, SQLite error code or other code on failure
 *
 */
int tasks_db_delete_entry_status(database *db, enum task_status status);

/**
 * 
 * @brief Retrieves a task record
 *
 * Fetches the complete record for the Tasks table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the task
 * @param[out] out_task struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int tasks_db_get(database *db, int id, struct task *out_task);

/**
 * @brief Gets the count of registered tasks in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of tasks on success, -1 on failure
 */
int tasks_db_get_count(database *db);

/**
 * @brief Writes all tasks records as a formatted string into provided buffer
 *
 * Executes a database query and formats all tasks records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 * 
 * @note Header will always needs 492 (493 (+\0) if no other row) bytes and each row + separator
 *       (Considering max input is 256) will need at max 1029 (last row 1030 for the '\0') with the current table and format
 *       String format:
 * +-----------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Title            | Description                      | Due Date    | Priority | Status      | Assigned To      | Created At          | Completed At        |
 * +-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+
 * | 1   | Restock Milk     | Restock milk on area area_name   | 2025-05-01  | Normal   | Done        | Alice            | 2025-06-02 17:08:18 | 2025-06-02 16:37:26 |
 * +-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+
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
int tasks_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @deprecated
 * 
 * @brief Retrieves all tasks records as a formatted string
 *
 * Executes a database query and formats all tasks records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +-----------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Title            | Description                      | Due Date    | Priority | Status      | Assigned To      | Created At          | Completed At        |
 * +-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+
 * | 1   | Restock Milk     | Restock milk on area area_name   | 2025-05-01  | Normal   | Done        | Alice            | 2025-06-02 17:08:18 | 2025-06-02 16:37:26 |
 * +-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+
 *
 * @note Header will always needs xxx bytes and each row + separator (xxx) will need at max xxxx with the current table and format
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
char *tasks_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all Tasks records
 *
 * Fetches all records from the Tasks table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int tasks_db_get_all(database *db);

#endif // TASKS_DB_H

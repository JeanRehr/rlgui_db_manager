/**
 * @file resident_db.h
 * @brief Resident Database Operations
 *
 * This header defines operations for managing resident records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of resident information.
 */

#ifndef RESIDENT_DB_H
#define RESIDENT_DB_H

#include <stddef.h>

#include "db_manager.h"
#include "resident.h"

/**
 * @brief Creates the Resident table in the database
 *
 * Creates a new Resident table if it doesn't already exist. The table includes fields for
 * CPF, name, age, health status, needs, medical assistance requirement, gender, and entry date.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int resident_db_create_table(database *db);

/**
 * @brief Inserts a new resident record into the database
 *
 * Adds a new record to the Resident table with the provided parameters.
 * The entry date is automatically set to the current date.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf Resident's CPF
 * @param[in] name Resident's full name
 * @param[in] age Resident's age
 * @param[in] health_status Description of health status
 * @param[in] needs Special needs or requirements
 * @param[in] medical_assistance Whether medical assistance is required
 * @param[in] gender Gender (0=Other, 1=Male, 2=Female)
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note The entry date is automatically set to the current date
 */
int resident_db_insert(
    database *db,
    const char *cpf,
    const char *name,
    int age,
    const char *health_status,
    const char *needs,
    bool medical_assistance,
    int gender
);

/**
 * @brief Updates an existing resident record
 *
 * Modifies the fields of an existing resident record identified by CPF.
 * Empty strings or invalid values for fields will preserve the existing values.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf CPF of the resident to update
 * @param[in] name_input New name (empty string preserves current)
 * @param[in] age_input New age (<= 0 preserves current)
 * @param[in] health_status_input New health status (empty string preserves current)
 * @param[in] needs_input New needs (empty string preserves current)
 * @param[in] medical_assistance_input New medical assistance status (-1 preserves current)
 * @param[in] gender_input New gender (< 0 preserves current)
 * @return SQLITE_OK on success, SQLite error code on failure
 */
int resident_db_update(
    database *db,
    const char *cpf,
    const char *name_input,
    int age_input,
    const char *health_status_input,
    const char *needs_input,
    int medical_assistance_input,
    int gender_input
);

/**
 * @brief Deletes a resident record by CPF
 *
 * Removes the resident record with the specified CPF from the database.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf CPF of the resident to delete
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if resident doesn't exist, or other SQLite error code
 */
int resident_db_delete_by_cpf(database *db, const char *cpf);

/**
 * @brief Checks if a CPF exists in the database
 *
 * Verifies whether a resident with the specified CPF exists in the database.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf CPF to check for existence
 * @return true if the resident exists, false otherwise
 */
bool resident_db_check_cpf_exists(database *db, const char *cpf);

/**
 * @brief Retrieves a resident record by CPF
 *
 * Fetches the complete record for the resident with the specified CPF.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf CPF of the resident to retrieve
 * @param[out] resident Pointer to structure where the data will be stored
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if resident doesn't exist, or other SQLite error code
 */
int resident_db_get_by_cpf(database *db, const char *cpf, struct resident *resident);

/**
 * @brief Gets the count of registered residents in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of residents on success, -1 on failure
 */
int resident_db_get_count(database *db);

/**
 * @brief Writes all resident records as a formatted string into provided buffer
 *
 * Executes a database query and formats all resident records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * @note Header will always needs 601 bytes and each row + separator (Considering max input is 256)
 *       will need at max 1040 with the current table and format
 *       String format:
 * +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | CPF         | Name                                       | Age | HealthStatus                               | Needs                                      | Medical Assistance | Gender | Entry Date |
 * +-------------+--------------------------------------------+-----+--------------------------------------------+--------------------------------------------+--------------------+--------+------------+
 * | 12345678901 | John Doe                                   | 30  | Healthy                                    | None                                       | False              | Other  | 2000-01-01 |
 * +-------------+--------------------------------------------+-----+--------------------------------------------+--------------------------------------------+--------------------+--------+------------+
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
int resident_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @brief Retrieves all resident records as a formatted string
 *
 * Executes a database query and formats all resident records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
 * | CPF         | Name                                       | Age | HealthStatus                               | Needs                                      | Medical Assistance | Gender | Entry Date |
 * +-------------+--------------------------------------------+-----+--------------------------------------------+--------------------------------------------+--------------------+--------+------------+
 * | 12345678901 | John Doe                                   | 30  | Healthy                                    | None                                       | False              | Other  | 2000-01-01 |
 * +-------------+--------------------------------------------+-----+--------------------------------------------+--------------------------------------------+--------------------+--------+------------+
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
 * @deprecated
 */
char *resident_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all resident records
 *
 * Fetches all records from the Resident table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int resident_db_get_all(database *db);

#endif // RESIDENT_DB_H

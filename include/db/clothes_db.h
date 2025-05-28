/**
 * @file clothes_db.h
 * @brief Clothes Database Operations
 *
 * This header defines operations for managing clothes records in an SQLite database,
 * including creation, insertion, updating, deletion, and querying of clothes information.
 */

#ifndef CLOTHES_DB_H
#define CLOTHES_DB_H

#include <stddef.h>

#include "db_manager.h"

#include "entities/clothing.h" /* To get the definition of the enums */

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

/**
 * @brief Inserts/updates a new clothes record into the database
 *
 * Adds a new record to the Clothes table with the provided parameters.
 * If the record already exists, then it updates the quantity.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] type Type of clothing (socks, coat, tshirt...)
 * @param[in] size Clothing's size (M, L, G...)
 * @param[in] gender Gender (Unissex, Male, Female)
 * @param[in] color Primary color the Clothing is made of (Black, Red, Multicolored, Patterned...)
 * @param[in] condition If it is new, good, worn, needs repair
 * @param[in] quantity Quantity being inserted, or to update
 * @param[in] notes General notes if needed
 *
 * @note Param notes can be null, then it will not be updated
 * 
 * @warning Should follow the same convention as the clothing struct on clothing.h
 *          type, size, gender, color and condition are enums from the defition of clothing struct
 *
 * @return SQLITE_OK on success, SQLITE_CONSTRAINT on quantity less than 0, SQLite error code on failure
 *
 */
int clothes_db_upsert(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity,
    const char *notes
);

/**
 * @brief Checks if a Clothes records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] type Type of clothing (socks, coat, tshirt...)
 * @param[in] size Clothing's size (M, L, G...)
 * @param[in] gender Gender (Unissex, Male, Female)
 * @param[in] color Primary color the Clothing is made of (Black, Red, Multicolored, Patterned...)
 * @param[in] condition If it is new, good, worn, needs repair
 * 
 * @warning Should follow the same convention as the clothing struct on clothing.h
 *          type, size, gender, color and condition are enums from the defition of clothing struct
 *
 * @return true if the record exists, false otherwise
 */
bool clothes_db_check_exists(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition
);

/**
 * @brief Checks if a Clothes records exists in the database
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the clothes to check
 *
 * @return true if the record exists, false otherwise
 */
bool clothes_db_check_exists_by_id(
    database *db,
    const int id
);

/**
 * @brief Removes a clothing from the Clothes table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] type Type of clothing (socks, coat, tshirt...)
 * @param[in] size Clothing's size (M, L, G...)
 * @param[in] gender Gender (Unissex, Male, Female)
 * @param[in] color Primary color the Clothing is made of (Black, Red, Multicolored, Patterned...)
 * @param[in] condition If it is new, good, worn, needs repair
 * @param[in] quantity_to_remove Quantity being to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the clothing doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 * @warning Should follow the same convention as the clothing struct on clothing.h
 *          type, size, gender, color and condition are enums from the defition of clothing struct
 *
 */
int clothes_db_remove(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity_to_remove
);

/**
 * @brief Removes a clothing from the Clothes table by a quantity amount
 *
 * Will not delete the table entry, only removes the quantity
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the clothes to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the clothing doesn't exist, SQLITE_CONSTRAINT if
 *         the quantity being removed will make the stock goes below 0 or the quantity to remove passed is below 0
 *         or other SQLite error code
 *
 */
int clothes_db_remove_by_id(
    database *db,
    const int id,
    const int quantity_to_remove
);

/**
 * @brief Deletes a clothes record by its UNIQUE data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] type Type of clothing (socks, coat, tshirt...)
 * @param[in] size Clothing's size (M, L, G...)
 * @param[in] gender Gender (Unissex, Male, Female)
 * @param[in] color Primary color the Clothing is made of (Black, Red, Multicolored, Patterned...)
 * @param[in] condition If it is new, good, worn, needs repair
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 * @warning Should follow the same convention as the clothing struct on clothing.h
 *          type, size, gender, color and condition are enums from the defition of clothing struct
 *
 */
int clothes_db_delete_entry(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition
);

/**
 * @brief Deletes a clothes record by its ID data
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] id ID of the clothes to remove the quantity
 * @param[in] quantity_to_remove Quantity to remove
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 */
int clothes_db_delete_entry_by_id(
    database *db,
    const int id
);

/**
 * 
 * @brief Retrieves a clothing record
 *
 * Fetches the complete record for the clothes table.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] type Type of clothing (socks, coat, tshirt...)
 * @param[in] size Clothing's size (M, L, G...)
 * @param[in] gender Gender (Unissex, Male, Female)
 * @param[in] color Primary color the Clothing is made of (Black, Red, Multicolored, Patterned...)
 * @param[in] condition If it is new, good, worn, needs repair
 * @param[out] clothing struct returned with the data
 *
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if the record doesn't exist, or other SQLite error code
 *
 * @warning Should follow the same convention as the clothing struct on clothing.h
 *          type, size, gender, color and condition are enums from the defition of clothing struct
 *
 */
int clothes_db_get(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    struct clothing *clothing
);

/**
 * @brief Gets the count of registered clothes in the database
 * 
 * @param db Pointer to initialized database structure
 * @return Total number of clothes on success, -1 on failure
 */
int clothes_db_get_count(database *db);

/**
 * @brief Writes all clothes records as a formatted string into provided buffer
 *
 * Executes a database query and formats all clothes records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * @note Header will always needs 370 bytes and each row + separator (Considering max input is 256)
 *       will need at max 470 with the current table and format
 *       String format:
 * +------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Type       | Size | Gender | Color        | Condition    | Quantity | Notes                                      |
 * +-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+
 * | 10  | tshirt     | l    | female | multicolored | worn         | 10       | zara brand, special donation               |
 * +-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+
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
int clothes_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @deprecated
 * 
 * @brief Retrieves all clothes records as a formatted string
 *
 * Executes a database query and formats all clothes records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +------------------------------------------------------------------------------------------------------------------------+
 * | ID  | Type       | Size | Gender | Color        | Condition    | Quantity | Notes                                      |
 * +-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+
 * | 10  | tshirt     | l    | female | multicolored | worn         | 10       | zara brand, special donation               |
 * +-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+
 * 
 * @note Header will always need 370 bytes and each row + separator will need at max 450 with the current table and format
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
char *clothes_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all clothes records
 *
 * Fetches all records from the Clothes table and displays them in a formatted table.
 * Primarily intended for debugging and administrative purposes.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @note Output is printed directly to stdout in table format
 */
int clothes_db_get_all(database *db);

#endif // CLOTHES_DB_H

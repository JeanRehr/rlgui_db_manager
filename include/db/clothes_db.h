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

#endif // CLOTHES_DB_H
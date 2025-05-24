/**
 * @file clothes_db.c
 * @brief Clothes database operations implementation
 */

#include "db/clothes_db.h"

#include <stdio.h>

int clothes_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Clothes ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Type TEXT NOT NULL,"                             // e.g. "t-shirt", "pants", "coat"
        "Size TEXT,"                                      // e.g. "M", "L", "XL", or "42", "kids"
        "Gender TEXT,"                                    // e.g. "other", "male", "female" NULL permitted for flexibility
        "Color TEXT,"                                     // e.g. "blue", "black", "red"
        "Quantity INTEGER NOT NULL DEFAULT 0,"            // Items in stock
        "Condition TEXT,"                                 // e.g. "new", "good", "worn", "needs repair"
        "Notes TEXT,"                                     // For arbitrary tracking (e.g., donor, special handling)
        "UNIQUE(Type, Size, Gender, Color, Condition));"; // Prevents accidental duplicate entries of the same clothes type
                                                          // e.g., multiple "t-shirt M male black new"

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Clothes table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}
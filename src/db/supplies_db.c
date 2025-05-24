/**
 * @file supplies_db.c
 * @brief Supplies database operations implementation
 */

#include "db/supplies_db.h"

#include <stdio.h>

int supplies_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Supplies ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"                  // e.g. "diaper", "tampon"
        "Category TEXT,"                       // e.g. "hygiene", "cleaning", "personal care"
        "Size TEXT,"                           // e.g. "adult", "small", "XXL"
        "Unit TEXT,"                           // e.g. "piece", "pack", "box"
        "Quantity INTEGER NOT NULL DEFAULT 0," // Items in stock
        "Notes TEXT,"                          // For arbitrary tracking
        "UNIQUE(Name, Category, Size));";      // Prevents accidental duplicate entries of the same supplies type
                                               // e.g., "multiple diaper hygiene M"

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Supplies table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}
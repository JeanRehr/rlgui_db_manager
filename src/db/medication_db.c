/**
 * @file medication_db.c
 * @brief Medication database operations implementation
 */

#include "db/medication_db.h"

#include <stdio.h>

int medication_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Medications ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"               // e.g. "Paracetamol 500g"
        "GenericName TEXT,"                 // e.g. "Paracetamol"
        "Form TEXT,"                        // e.g. "Tablet", "Syrup", "Injection"
        "Strength TEXT,"                    // e.g. "500mg", "5mg/ml"
        "Unit TEXT,"                        // e.g. "Tablet", "ml", "vial"
        "Stock INTEGER NOT NULL DEFAULT 0," // Current count in inventory
        "ExpirationDate TEXT,"              // Soonest expiration date
        "Notes TEXT,"                       // General notes if needed
        "UNIQUE(Name, Form, Strength));";   // Prevents accidental duplicate entries of the same medication in the same
                                            // dosage and form e.g. multiple "Paracetamol 500mg Tablet"

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Medications table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}
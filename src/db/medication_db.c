/**
 * @file medication_db.c
 * @brief Medication database operations implementation
 */

#include "db/medication_db.h"

#include <stdio.h>
#include <string.h>

int medication_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Medications ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"
        "GenericName TEXT,"
        "Form TEXT NOT NULL,"
        "Strength TEXT NOT NULL,"
        "Unit TEXT,"
        "Stock INTEGER NOT NULL DEFAULT 0,"
        "ExpirationDate TEXT,"
        "Notes TEXT,"
        "UNIQUE(Name, Form, Strength)"
        ");";

    /**
     * e.g. "Paracetamol 500g"
     * e.g. "Paracetamol"
     * e.g. "Tablet", "Syrup", "Injection"
     * e.g. "500mg", "5mg/ml"
     * e.g. "Tablet", "ml", "vial"
     * Current count in inventory
     * Soonest expiration date
     * General notes if needed
     * Prevents accidental duplicate entries of the same medication in the same
     * dosage and form e.g. multiple "Paracetamol Tablet 500mg"
     */

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Medications table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}

int medication_db_upsert(
    database *db,
    const char *name,
    const char *generic_name,
    const char *form,
    const char *strength,
    const char *unit,
    const int stock,
    const char *expiration_date,
    const char *notes
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }
    if (stock < 0) {
        fprintf(stderr, "Stock cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql =
        "INSERT INTO Medications "
        "(Name, GenericName, Form, Strength, Unit, Stock, ExpirationDate, Notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(Name, Form, Strength) DO UPDATE SET "
        "Stock = Stock + excluded.Stock, "
        "ExpirationDate = COALESCE(excluded.ExpirationDate, Medications.ExpirationDate), "
        "Notes = COALESCE(excluded.Notes, Medications.Notes);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, generic_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, form, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, strength, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, unit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, stock);

    if (expiration_date) {// Only substitute expiration date if it is not null
        sqlite3_bind_text(stmt, 7, expiration_date, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 7);
    }

    if (notes) { // Only substitute notes if it is not null
        sqlite3_bind_text(stmt, 8, notes, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

bool medication_db_check_exists(database *db, const char *name, const char *form, const char *strength) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Medications WHERE Name=? AND Form=? AND Strength=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // Bind params
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, form, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, strength, -1, SQLITE_STATIC);

    bool exists = false;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        exists = true; // A row was found with the given data
    } else if (rc != SQLITE_DONE) {
        fprintf(
            stderr,
            "Failed to execute statement on function %s, line %d: %s\n",
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
    }

    sqlite3_finalize(stmt);
    return exists;
}

bool medication_db_check_exists_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Medications WHERE ID=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // Bind params
    sqlite3_bind_int(stmt, 1, id);

    bool exists = false;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        exists = true; // A row was found with the given data
    } else if (rc != SQLITE_DONE) {
        fprintf(
            stderr,
            "Failed to execute statement on function %s, line %d: %s\n",
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
    }

    sqlite3_finalize(stmt);
    return exists;
}

int medication_db_remove(
    database *db,
    const char *name,
    const char *form,
    const char *strength,
    const int quantity_to_remove
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity_to_remove < 0) {
        fprintf(stderr, "Stock cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    if (!medication_db_check_exists(db, name, form, strength)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Medications SET Stock=Stock-? "
        "WHERE Name=? AND Form=? AND Strength=? AND Stock >= ?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, quantity_to_remove);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, form, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, strength, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, quantity_to_remove);

    rc = sqlite3_step(stmt);

    int rows = sqlite3_changes(db->db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Not possible to remove medication quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int medication_db_remove_by_id(database *db, const int id, const int quantity_to_remove) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity_to_remove < 0) {
        fprintf(stderr, "Stock cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    if (!medication_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Medications SET Stock=Stock-? "
        "WHERE ID=? AND Stock >= ?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, quantity_to_remove);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_bind_int(stmt, 3, quantity_to_remove);

    rc = sqlite3_step(stmt);

    int rows = sqlite3_changes(db->db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Not possible to remove medication quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int medication_db_delete_entry(database *db, const char *name, const char *form, const char *strength) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!medication_db_check_exists(db, name, form, strength)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Medications WHERE Name=? AND Form=? AND Strength=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, form, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, strength, -1, SQLITE_STATIC);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int medication_db_delete_entry_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!medication_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Medications WHERE ID=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, id);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int medication_db_get(
    database *db,
    const char *name,
    const char *form,
    const char *strength,
    struct medication *medication
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT Name, GenericName, Form, Strength, Unit, Stock, ExpirationDate, Notes FROM Medications WHERE Name=? AND Form=? AND Strength=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, form, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, strength, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strcpy(medication->name, (const char *)sqlite3_column_text(stmt, 0));

        const char *generic_name_val = (const char *)sqlite3_column_text(stmt, 1);
        if (generic_name_val) {
            strcpy(medication->generic_name, generic_name_val);
        } else {
            medication->generic_name[0] = '\0';
        }

        strcpy(medication->form, (const char *)sqlite3_column_text(stmt, 2));

        strcpy(medication->strength, (const char *)sqlite3_column_text(stmt, 3));
        
        const char *unit_val = (const char *)sqlite3_column_text(stmt, 4);
        if (unit_val) {
            strcpy(medication->unit, unit_val);
        } else {
            medication->unit[0] = '\0';
        }
        
        medication->stock = sqlite3_column_int(stmt, 5);
        
        const char *expiration_date_val = (const char *)sqlite3_column_text(stmt, 6);
        if (expiration_date_val) {
            strcpy(medication->expiration_date, expiration_date_val);
        } else {
            medication->expiration_date[0] = '\0';
        }

        const char *note_val = (const char *)sqlite3_column_text(stmt, 7);
        if (note_val) {
            strcpy(medication->notes, note_val);
        } else {
            medication->notes[0] = '\0';
        }

        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No medication found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int medication_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Medications;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }

    int count = 0;

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

int medication_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM Medications;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf(
        "+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Name                     | GenericName              | Form             | Strength         | Unit                     | Stock | ExpirationDate | Notes                    |\n"
        "+-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *generic_name = (const char *)sqlite3_column_text(stmt, 2);
        const char *form = (const char *)sqlite3_column_text(stmt, 3);
        const char *strength = (const char *)sqlite3_column_text(stmt, 4);
        const char *unit = (const char *)sqlite3_column_text(stmt, 5);
        const int stock = sqlite3_column_int(stmt, 6);
        const char *expiration_date = (const char *)sqlite3_column_text(stmt, 7);
        const char *notes = (const char *)sqlite3_column_text(stmt, 8);

        printf(
            "| %-3d | %-24s | %-24s | %-16s | %-16s | %-24s | %-5d | %-14s | %-24s |\n",
            id,
            name,
            generic_name,
            form,
            strength,
            unit,
            stock,
            expiration_date,
            notes
        );
        printf(
        "+-----+--------------------------+--------------------------+------------------+------------------+--------------------------+-------+----------------+--------------------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

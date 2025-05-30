/**
 * @file supplies_db.c
 * @brief Supplies database operations implementation
 */

#include "db/supplies_db.h"

#include <stdio.h>
#include <string.h>

int supplies_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Supplies ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"               // e.g. "diaper", "tampon"
        "Category TEXT,"                    // e.g. "hygiene", "cleaning", "personal care"
        "Size TEXT,"                        // e.g. "adult", "small", "XXL"
        "Unit TEXT,"                        // e.g. "piece", "pack", "box"
        "Stock INTEGER NOT NULL DEFAULT 0," // Items in stock
        "Notes TEXT,"                       // For arbitrary tracking
        "UNIQUE(Name, Category, Size));";   // Prevents accidental duplicate entries of the same supplies type
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

int supplies_db_upsert(
    database *db,
    const char *name,
    const char *category,
    const char *size,
    const char *unit,
    const int stock,
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
        "INSERT INTO Supplies "
        "(Name, Category, Size, Unit, Stock, Notes) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(Name, Category, Size) DO UPDATE SET "
        "Stock = Stock + excluded.Stock, "
        "Notes = COALESCE(excluded.Notes, Supplies.Notes);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, size, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, unit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, stock);

    if (notes) { // Only substitute notes if it is not null
        sqlite3_bind_text(stmt, 6, notes, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 6);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

bool supplies_db_check_exists(database *db, const char *name, const char *category, const char *unit) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Supplies WHERE Name=? AND Category=? AND Unit=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // Bind params
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, unit, -1, SQLITE_STATIC);

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

bool supplies_db_check_exists_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Supplies WHERE ID=?;";

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

int supplies_db_remove(
    database *db,
    const char *name,
    const char *category,
    const char *unit,
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

    if (!supplies_db_check_exists(db, name, category, unit)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Supplies SET Stock=Stock-? "
        "WHERE Name=? AND Category=? AND Unit=? AND Stock >= ?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, quantity_to_remove);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, unit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, quantity_to_remove);

    rc = sqlite3_step(stmt);

    int rows = sqlite3_changes(db->db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Not possible to remove supply quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int supplies_db_remove_by_id(database *db, const int id, const int quantity_to_remove) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity_to_remove < 0) {
        fprintf(stderr, "Stock cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    if (!supplies_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Supplies SET Stock=Stock-? "
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
        fprintf(stderr, "Not possible to remove supply quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int supplies_db_delete_entry(database *db, const char *name, const char *category, const char *unit) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!supplies_db_check_exists(db, name, category, unit)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Supplies WHERE Name=? AND Category=? AND Unit=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, unit, -1, SQLITE_STATIC);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int supplies_db_delete_entry_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!supplies_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Supplies WHERE ID=?;";

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

int supplies_db_get(database *db, const char *name, const char *category, const char *unit, struct supply *out_supply) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT Name, Category, Size, Unit, Stock, Notes FROM Supplies WHERE Name=? AND Category=? AND Unit=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, unit, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strcpy(out_supply->name, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(out_supply->category, (const char *)sqlite3_column_text(stmt, 1));

        const char *size_val = (const char *)sqlite3_column_text(stmt, 2);
        if (size_val) {
            strcpy(out_supply->size, size_val);
        } else {
            out_supply->size[0] = '\0';
        }

        strcpy(out_supply->unit, (const char *)sqlite3_column_text(stmt, 3));

        out_supply->stock = sqlite3_column_int(stmt, 4);

        const char *note_val = (const char *)sqlite3_column_text(stmt, 5);
        if (note_val) {
            strcpy(out_supply->notes, note_val);
        } else {
            out_supply->notes[0] = '\0';
        }

        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No supply found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int supplies_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Supplies;";

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

int supplies_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM Supplies;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf(
        "+------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Name                     | Category                 | Size             | Unit             | Stock | Notes                    |\n"
        "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *category = (const char *)sqlite3_column_text(stmt, 2);
        const char *size = (const char *)sqlite3_column_text(stmt, 3);
        const char *unit = (const char *)sqlite3_column_text(stmt, 4);
        const int stock = sqlite3_column_int(stmt, 5);
        const char *notes = (const char *)sqlite3_column_text(stmt, 6);

        printf(
            "| %-3d | %-24s | %-24s | %-16s | %-16s | %-5d | %-24s |\n",
            id,
            name,
            category,
            size,
            unit,
            stock,
            notes
        );
        printf(
            "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

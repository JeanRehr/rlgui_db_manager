/**
 * @file supplies_db.c
 * @brief Supplies database operations implementation
 */

#include "db/supplies_db.h"

#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)
#include <stdio.h>
#include <stdlib.h>
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

bool supplies_db_check_exists(database *db, const char *name, const char *category, const char *size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Supplies WHERE Name=? AND Category=? AND Size=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // Bind params
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, size, -1, SQLITE_STATIC);

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
    const char *size,
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

    if (!supplies_db_check_exists(db, name, category, size)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Supplies SET Stock=Stock-? "
        "WHERE Name=? AND Category=? AND Size=? AND Stock >= ?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, quantity_to_remove);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, size, -1, SQLITE_STATIC);
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

int supplies_db_delete_entry(database *db, const char *name, const char *category, const char *size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!supplies_db_check_exists(db, name, category, size)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Supplies WHERE Name=? AND Category=? AND Size=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, size, -1, SQLITE_STATIC);

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

int supplies_db_get(database *db, const char *name, const char *category, const char *size, struct supply *out_supply) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT Name, Category, Size, Unit, Stock, Notes FROM Supplies WHERE Name=? AND Category=? AND Size=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, category, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, size, -1, SQLITE_STATIC);

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

int supplies_db_get_by_id(database *db, const int id, struct supply *out_supply) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT Name, Category, Size, Unit, Stock, Notes FROM Supplies WHERE ID=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, id);

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

int supplies_db_get_all_format(database *db, char *buffer, size_t buffer_size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    if (!buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return -1;
    }

    const char *sql = "SELECT * FROM Supplies;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }

    // Initialize buffer with empty string
    buffer[0] = '\0';
    size_t written = 0;

    // Format header
    const char *header =
        "+------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Name                     | Category                 | Size             | Unit             | Stock | Notes                    |\n"
        "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n";

    // Write header if there's space
    size_t header_len = strlen(header);
    if (written + header_len < buffer_size) {
        strcpy(buffer + written, header);
        written += header_len;
    } else {
        // Truncate but ensure null termination
        if (buffer_size > 0) {
            strncpy(buffer, header, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
        }
        sqlite3_finalize(stmt);
        return -1;
    }

    // Process each row
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *category = (const char *)sqlite3_column_text(stmt, 2);
        const char *size = (const char *)sqlite3_column_text(stmt, 3);
        const char *unit = (const char *)sqlite3_column_text(stmt, 4);
        const int stock = sqlite3_column_int(stmt, 5);
        const char *notes = (const char *)sqlite3_column_text(stmt, 6);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-24s | %-24s | %-16s | %-16s | %-5d | %-24s |\n",
            id,
            name,
            category,
            size,
            unit,
            stock,
            notes
        );

        size_t row_len = strlen(row);
        if (written + row_len < buffer_size) {
            strcpy(buffer + written, row);
            written += row_len;
        } else {
            // Truncate but ensure null termination
            if (buffer_size > 0) {
                strncpy(buffer, row, buffer_size - 1);
                buffer[buffer_size - 1] = '\0';
            }
            sqlite3_finalize(stmt);
            return -1;
        }

        // Add separator line
        const char *separator =
            "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n";

        size_t separator_len = strlen(separator);
        if (written + separator_len < buffer_size) {
            strcpy(buffer + written, separator);
            written += separator_len;
        } else {
            // Truncate but ensure null termination
            if (buffer_size > 0) {
                strncpy(buffer, separator, buffer_size - 1);
                buffer[buffer_size - 1] = '\0';
            }
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return written;
}

char *supplies_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT * FROM Supplies;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

    // Header will always needs 405 (406 (\0) if no other row) bytes and each row + separator will need at max 1450 with the current table and format

    // Initial buffer
    size_t buffer_size = 4096;
    char *result = malloc(buffer_size);
    if (!result) {
        fprintf(stderr, "Memory allocation failed.\n");
        sqlite3_finalize(stmt);
        return NULL;
    }
    result[0] = '\0';                     // Initialize empty string
    size_t total_allocated = buffer_size; // Track total bytes allocated

    // Append header to the result string
    const char *header =
        "+------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Name                     | Category                 | Size             | Unit             | Stock | Notes                    |\n"
        "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n";

    // Check if buffer is large enough for the header
    if (strlen(header) + 1 > buffer_size) {
        buffer_size = strlen(header) + 1;
        result = realloc(result, buffer_size);
        printf("REALLOC CALLED DURING HEADER!\n");
        if (!result) {
            fprintf(stderr, "Memory reallocation failed.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }
        total_allocated += (buffer_size - total_allocated); // Update total allocated
    }
    strcpy(result, header);

    // Process each row
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        const char *category = (const char *)sqlite3_column_text(stmt, 2);
        const char *size = (const char *)sqlite3_column_text(stmt, 3);
        const char *unit = (const char *)sqlite3_column_text(stmt, 4);
        const int stock = sqlite3_column_int(stmt, 5);
        const char *notes = (const char *)sqlite3_column_text(stmt, 6);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-24s | %-24s | %-16s | %-16s | %-5d | %-24s |\n",
            id,
            name,
            category,
            size,
            unit,
            stock,
            notes
        );

        // Check if buffer needs to grow
        size_t required_size = strlen(result) + strlen(row) + 1;
        if (required_size > buffer_size) {
            size_t old_size = buffer_size;
            buffer_size = required_size * 2; // Double the buffer to reduce realloc calls
            char *new_result = realloc(result, buffer_size);
            printf("REALLOC CALLED DURING ROW FORMATTING!\n");
            if (!new_result) {
                fprintf(stderr, "Memory reallocation failed.\n");
                free(result);
                sqlite3_finalize(stmt);
                return NULL;
            }
            result = new_result;
            total_allocated += (buffer_size - old_size); // Update total allocated
        }

        // Append the row to the result
        strcat(result, row);

        // Add separator line
        const char *separator =
            "+-----+--------------------------+--------------------------+------------------+------------------+-------+--------------------------+\n";

        required_size = strlen(result) + strlen(separator) + 1;
        if (required_size > buffer_size) {
            size_t old_size = buffer_size;
            buffer_size = required_size * 2;
            char *new_result = realloc(result, buffer_size);
            printf("REALLOC CALLED DURING SEPARATOR!\n");
            if (!new_result) {
                fprintf(stderr, "Memory reallocation failed.\n");
                free(result);
                sqlite3_finalize(stmt);
                return NULL;
            }
            result = new_result;
            total_allocated += (buffer_size - old_size); // Update total allocated
        }
        strcat(result, separator);
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
        free(result);
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_finalize(stmt);
    printf("Total bytes allocated: %" PRIu64 "\n", total_allocated);
    return result; // Caller must free() this memory!
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

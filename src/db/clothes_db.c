/**
 * @file clothes_db.c
 * @brief Clothes database operations implementation
 */

#include "db/clothes_db.h"

#include <inttypes.h> // For PRIu64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int clothes_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Clothes ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Type INTEGER NOT NULL,"                          // e.g. "t-shirt", "pants", "coat"
        "Size INTEGER,"                                   // e.g. "xs", "s", "m", "l"
        "Gender INTEGER,"                                 // e.g. "other", "male", "female"
        "Color INTEGER,"                                  // e.g. "blue", "black", "red"
        "Condition INTEGER,"                              // e.g. "new", "good", "worn", "needs repair"
        "Quantity INTEGER NOT NULL DEFAULT 0,"            // Items in stock
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

static int _clothes_db_upsert(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity,
    const char *notes
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity < 0) {
        fprintf(stderr, "Quantity cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql =
        "INSERT INTO Clothes "
        "(Type, Size, Gender, Color, Condition, Quantity, Notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(Type, Size, Gender, Color, Condition) DO UPDATE SET "
        "Quantity = Quantity + excluded.Quantity, "
        "Notes = COALESCE(excluded.Notes, Clothes.Notes);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind params
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, size);
    sqlite3_bind_int(stmt, 3, gender);
    sqlite3_bind_int(stmt, 4, color);
    sqlite3_bind_int(stmt, 5, condition);
    sqlite3_bind_int(stmt, 6, quantity);
    if (notes) // Only substitute notes if it is not null
        sqlite3_bind_text(stmt, 7, notes, -1, SQLITE_STATIC);
    else
        sqlite3_bind_null(stmt, 7);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

int clothes_db_upsert(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity,
    const char *notes
) {
    int rc = _clothes_db_upsert(db, type, size, gender, color, condition, quantity, notes);

    if (rc == SQLITE_OK) {
        if (db->logger) {
            logger_log(
                db->logger,
                "Inserted Clothing with type: [%s], size: [%s], gender: [%s], color: [%s], condition: [%s] by quantity %d",
                clothing_type_str[type],
                clothing_size_str[size],
                clothing_gender_str[gender],
                clothing_color_str[color],
                clothing_condition_str[condition],
                quantity
            );
        }
    }

    return rc;
}

bool clothes_db_check_exists(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Clothes WHERE Type=? AND Size=? AND Gender=? AND Color=? AND Condition=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    // Bind params
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, size);
    sqlite3_bind_int(stmt, 3, gender);
    sqlite3_bind_int(stmt, 4, color);
    sqlite3_bind_int(stmt, 5, condition);

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

bool clothes_db_check_exists_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Clothes WHERE ID=?;";

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

static int _clothes_db_remove(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity_to_remove
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity_to_remove < 0) {
        fprintf(stderr, "Quantity cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    if (!clothes_db_check_exists(db, type, size, gender, color, condition)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql =
        "UPDATE Clothes SET Quantity=Quantity-? "
        "WHERE Type=? AND Size=? AND Gender=? AND Color=? AND Condition=? AND Quantity >= ?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Not possible to prepare sql statement: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, quantity_to_remove);
    sqlite3_bind_int(stmt, 2, type);
    sqlite3_bind_int(stmt, 3, size);
    sqlite3_bind_int(stmt, 4, gender);
    sqlite3_bind_int(stmt, 5, color);
    sqlite3_bind_int(stmt, 6, condition);
    sqlite3_bind_int(stmt, 7, quantity_to_remove);

    rc = sqlite3_step(stmt);

    int rows = sqlite3_changes(db->db);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Not possible to remove clothes quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int clothes_db_remove(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    const int quantity_to_remove
) {
    int rc = _clothes_db_remove(db, type, size, gender, color, condition, quantity_to_remove);

    if (rc == SQLITE_OK) {
        if (db->logger) {
            logger_log(
                db->logger,
                "Removed Clothing with type: [%s], size: [%s], gender: [%s], color: [%s], condition: [%s], by quantity %d",
                clothing_type_str[type],
                clothing_size_str[size],
                clothing_gender_str[gender],
                clothing_color_str[color],
                clothing_condition_str[condition],
                quantity_to_remove
            );
        }
    }

    return rc;
}

static int _clothes_db_remove_by_id(database *db, const int id, const int quantity_to_remove) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity_to_remove < 0) {
        fprintf(stderr, "Quantity cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    if (!clothes_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record doesn't exists.\n");
        return SQLITE_NOTFOUND; // Row not found
    }

    const char *update_sql = "UPDATE Clothes SET Quantity=Quantity-? WHERE ID=? AND Quantity >= ?;";

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
        fprintf(stderr, "Not possible to remove clothes quantity: %s.\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (rows > 0) {
        return SQLITE_OK;
    }

    // If we get here it means not enough stock
    fprintf(stderr, "Stock quantity cannot go below 0.\n");
    return SQLITE_CONSTRAINT;
}

int clothes_db_remove_by_id(database *db, const int id, const int quantity_to_remove) {
    struct clothing removed_clothing = { 0 };
    int rc = _clothes_db_remove_by_id(db, id, quantity_to_remove);

    if (rc == SQLITE_OK) {
        if (db->logger) {
            logger_log(
                db->logger,
                "Removed Clothing with ID %d, type: [%s], size: [%s], gender: [%s], color: [%s], condition: [%s], by quantity %d",
                id,
                clothing_type_str[removed_clothing.type],
                clothing_size_str[removed_clothing.size],
                clothing_gender_str[removed_clothing.gender],
                clothing_color_str[removed_clothing.color],
                clothing_condition_str[removed_clothing.condition],
                quantity_to_remove
            );
        }
    }

    return rc;
}

static int _clothes_db_delete_entry(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!clothes_db_check_exists(db, type, size, gender, color, condition)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Clothes WHERE Type=? AND Size=? AND Gender=? AND Color=? AND Condition=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, size);
    sqlite3_bind_int(stmt, 3, gender);
    sqlite3_bind_int(stmt, 4, color);
    sqlite3_bind_int(stmt, 5, condition);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int clothes_db_delete_entry(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition
) {
    int rc = _clothes_db_delete_entry(db, type, size, gender, color, condition);

    if (rc == SQLITE_OK) {
        if (db->logger) {
            logger_log(
                db->logger,
                "Deleted Clothing entry with type: [%s], size: [%s], gender: [%s], color: [%s], condition: [%s].",
                clothing_type_str[type],
                clothing_size_str[size],
                clothing_gender_str[gender],
                clothing_color_str[color],
                clothing_condition_str[condition]
            );
        }
    }

    return rc;
}

static int _clothes_db_delete_entry_by_id(database *db, const int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!clothes_db_check_exists_by_id(db, id)) {
        fprintf(stderr, "Record not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Clothes WHERE ID=?;";

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

int clothes_db_delete_entry_by_id(database *db, const int id) {
    struct clothing deleted_clothing = { 0 };
    int rc = _clothes_db_delete_entry_by_id(db, id);

    if (rc == SQLITE_OK) {
        if (db->logger) {
            logger_log(
                db->logger,
                "Deleted Clothing entry with ID %d, type: [%s], size: [%s], gender: [%s], color: [%s], condition: [%s].",
                id,
                clothing_type_str[deleted_clothing.type],
                clothing_size_str[deleted_clothing.size],
                clothing_gender_str[deleted_clothing.gender],
                clothing_color_str[deleted_clothing.color],
                clothing_condition_str[deleted_clothing.condition]
            );
        }
    }

    return rc;
}

int clothes_db_get(
    database *db,
    const enum clothing_type type,
    const enum clothing_size size,
    const enum clothing_gender gender,
    const enum clothing_color color,
    const enum clothing_condition condition,
    struct clothing *clothing
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT Type, Size, Gender, Color, Condition, Quantity, Notes FROM Clothes WHERE Type=? AND Size=? AND Gender=? AND Color=? AND Condition=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the parameters
    sqlite3_bind_int(stmt, 1, type);
    sqlite3_bind_int(stmt, 2, size);
    sqlite3_bind_int(stmt, 3, gender);
    sqlite3_bind_int(stmt, 4, color);
    sqlite3_bind_int(stmt, 5, condition);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Type (0), Size (1), Gender (2), Color (3), Condition (4), Quantity (5), Notes (6)
        clothing->type = sqlite3_column_int(stmt, 0);
        clothing->size = sqlite3_column_int(stmt, 1);
        clothing->gender = sqlite3_column_int(stmt, 2);
        clothing->color = sqlite3_column_int(stmt, 3);
        clothing->condition = sqlite3_column_int(stmt, 4);
        clothing->quantity = sqlite3_column_int(stmt, 5);
        const char *note_val = (const char *)sqlite3_column_text(stmt, 6);
        if (note_val) {
            strcpy(clothing->notes, note_val);
        } else {
            clothing->notes[0] = '\0';
        }
        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No clothing found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int clothes_db_get_by_id(database *db, const int id, struct clothing *clothing) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT Type, Size, Gender, Color, Condition, Quantity, Notes FROM Clothes WHERE ID=?;";

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
        // Type (0), Size (1), Gender (2), Color (3), Condition (4), Quantity (5), Notes (6)
        clothing->type = sqlite3_column_int(stmt, 0);
        clothing->size = sqlite3_column_int(stmt, 1);
        clothing->gender = sqlite3_column_int(stmt, 2);
        clothing->color = sqlite3_column_int(stmt, 3);
        clothing->condition = sqlite3_column_int(stmt, 4);
        clothing->quantity = sqlite3_column_int(stmt, 5);
        const char *note_val = (const char *)sqlite3_column_text(stmt, 6);
        if (note_val) {
            strcpy(clothing->notes, note_val);
        } else {
            clothing->notes[0] = '\0';
        }
        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No clothing found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int clothes_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Clothes;";

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

int clothes_db_get_all_format(database *db, char *buffer, size_t buffer_size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    if (!buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return -1;
    }

    const char *sql = "SELECT * FROM Clothes;";
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
        "+------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Type       | Size | Gender | Color        | Condition    | Quantity | Notes                                      |\n"
        "+-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+\n";

    // Write header if there's space
    size_t header_len = strlen(header);
    if (written + header_len < buffer_size) {
        strcpy(buffer + written, header);
        written += header_len;
    } else {
        // Truncate but ensure null termination
        if (buffer_size > 0) {
            memcpy(buffer, header, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
        }
        sqlite3_finalize(stmt);
        return -1;
    }

    // Process each row
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        enum clothing_type type = sqlite3_column_int(stmt, 1);
        enum clothing_size size = sqlite3_column_int(stmt, 2);
        enum clothing_gender gender = sqlite3_column_int(stmt, 3);
        enum clothing_color color = sqlite3_column_int(stmt, 4);
        enum clothing_condition condition = sqlite3_column_int(stmt, 5);
        int quantity = sqlite3_column_int(stmt, 6);
        const char *notes = (const char *)sqlite3_column_text(stmt, 7);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-10s | %-4s | %-6s | %-12s | %-12s | %-8d | %-42s |\n",
            id,
            clothing_type_str[type],
            clothing_size_str[size],
            clothing_gender_str[gender],
            clothing_color_str[color],
            clothing_condition_str[condition],
            quantity,
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
            "+-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+\n";

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

char *clothes_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT ID, Type, Size, Gender, Color, Condition, Quantity, Notes FROM Clothes;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

    // Header will always needs 370 bytes (counting last \n+\o in case no rows) and each row + separator will need at max 450 (counting last \n+\o in case no more rows) with the current table and format

    // Initial buffer
    size_t buffer_size = 820;
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
        "+------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Type       | Size | Gender | Color        | Condition    | Quantity | Notes                                      |\n"
        "+-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+\n";

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
        enum clothing_type type = sqlite3_column_int(stmt, 1);
        enum clothing_size size = sqlite3_column_int(stmt, 2);
        enum clothing_gender gender = sqlite3_column_int(stmt, 3);
        enum clothing_color color = sqlite3_column_int(stmt, 4);
        enum clothing_condition condition = sqlite3_column_int(stmt, 5);
        int quantity = sqlite3_column_int(stmt, 6);
        const char *notes = (const char *)sqlite3_column_text(stmt, 7);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-10s | %-4s | %-6s | %-12s | %-12s | %-8d | %-42s |\n",
            id,
            clothing_type_str[type],
            clothing_size_str[size],
            clothing_gender_str[gender],
            clothing_color_str[color],
            clothing_condition_str[condition],
            quantity,
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
            "+-----+------------+------+--------+--------------+--------------+----------+----------------------------------+\n";

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

int clothes_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM Clothes;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf(
        "+------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Type       | Size | Gender | Color        | Condition    | Quantity | Notes                                      |\n"
        "+-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        enum clothing_type type = sqlite3_column_int(stmt, 1);
        enum clothing_size size = sqlite3_column_int(stmt, 2);
        enum clothing_gender gender = sqlite3_column_int(stmt, 3);
        enum clothing_color color = sqlite3_column_int(stmt, 4);
        enum clothing_condition condition = sqlite3_column_int(stmt, 5);
        int quantity = sqlite3_column_int(stmt, 6);
        const char *notes = (const char *)sqlite3_column_text(stmt, 7);

        printf(
            "| %-3d | %-10s | %-4s | %-6s | %-12s | %-12s | %-8d | %-42s |\n",
            id,
            clothing_type_str[type],
            clothing_size_str[size],
            clothing_gender_str[gender],
            clothing_color_str[color],
            clothing_condition_str[condition],
            quantity,
            notes
        );
        printf(
            "+-----+------------+------+--------+--------------+--------------+----------+--------------------------------------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

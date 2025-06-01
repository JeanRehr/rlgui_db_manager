/**
 * @file foodbatch_db.c
 * @brief Food batch database operations implementation
 */
#include "db/foodbatch_db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)

int foodbatch_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS FoodBatch ("
        "BatchId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Name TEXT NOT NULL,"            // Name of the food e.g. "Banana"
        "Quantity REAL NOT NULL,"        // For both weight and count
        "Unit TEXT NOT NULL,"            // e.g. "L", "KG", "Piece", "cans", "lbs"
        "IsPerishable INTEGER NOT NULL," // 0 or 1
        "ArrivalDate TEXT NOT NULL,"     // ISO format 2000-12-21
        "ExpirationDate TEXT);";         // Nullable, ISO format 2000-12-21

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init FoodBatch Table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}

int foodbatch_db_insert(
    database *db,
    const char *name,
    const float quantity,
    const char *unit,
    const bool is_perishable,
    const char *arrival_date,
    const char *expiration_date
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (quantity < 0) {
        fprintf(stderr, "Quantity cannot be less than 0.\n");
        return SQLITE_CONSTRAINT;
    }

    // SQL query to insert a new food batch
    const char *sql =
        "INSERT INTO FoodBatch "
        "(Name, Quantity, Unit, IsPerishable, ArrivalDate, ExpirationDate) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind values to the prepared statement
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, quantity);
    sqlite3_bind_text(stmt, 3, unit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_perishable ? 1 : 0);
    sqlite3_bind_text(stmt, 5, arrival_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, expiration_date, -1, SQLITE_STATIC);

    // Execute the SQL statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(
            stderr,
            "Failed to execute statement on function %s, line %d: %s\n",
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
    }

    // Finalize and clean up the prepared statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int foodbatch_db_update(
    database *db,
    const int batch_id,
    const char *name_input,
    const float quantity_input,
    const char *unit_input,
    const bool is_perishable_input,
    const char *arrival_date_input,
    const char *expiration_date_input
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    struct foodbatch foodbatch;
    int rc = foodbatch_db_get_by_batchid(db, batch_id, &foodbatch);

    if (rc != SQLITE_OK) {
        fprintf(
            stderr,
            "Not possible to get foodbatch by batchid: %d on function %s, line %d: %s\n",
            batch_id,
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
        return rc;
    }

    // Decide which fields to use for update based on inputs
    const char *name = (name_input[0] != '\0') ? name_input : foodbatch.name;
    float quantity = (quantity_input > 0) ? quantity_input : foodbatch.quantity;
    const char *unit = (unit_input[0] != '\0') ? unit_input : foodbatch.unit;
    int is_perishable = (is_perishable_input > 0) ? is_perishable_input : foodbatch.is_perishable;
    const char *arrival_date = (arrival_date_input[0] != '\0') ? arrival_date_input : foodbatch.arrival_date;
    const char *expiration_date =
        (expiration_date_input[0] != '\0') ? expiration_date_input : foodbatch.expiration_date;

    const char *sql =
        "UPDATE FoodBatch SET Name = ?, Quantity = ?, Unit = ?, IsPerishable = ?, ArrivalDate = ?, ExpirationDate = ? "
        "WHERE BatchId = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, quantity);
    sqlite3_bind_text(stmt, 3, unit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_perishable);
    sqlite3_bind_text(stmt, 5, arrival_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, expiration_date, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, batch_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(
            stderr,
            "Failed to execute statement on function %s, line %d: %s\n",
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int foodbatch_db_delete_by_id(database *db, int batch_id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!foodbatch_db_check_batchid_exists(db, batch_id)) {
        fprintf(stderr, "Batch ID not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM FoodBatch WHERE BatchId = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the BatchId parameter
    sqlite3_bind_int(stmt, 1, batch_id);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int foodbatch_db_get_by_batchid(database *db, int batch_id, struct foodbatch *out_foodbatch) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT BatchId, Name, Quantity, Unit, IsPerishable, ArrivalDate, ExpirationDate FROM "
        "FoodBatch WHERE BatchId = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, batch_id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out_foodbatch->batch_id = sqlite3_column_int(stmt, 0);
        strcpy(out_foodbatch->name, (const char *)sqlite3_column_text(stmt, 1));
        out_foodbatch->quantity = (float)sqlite3_column_double(stmt, 2);
        strcpy(out_foodbatch->unit, (const char *)sqlite3_column_text(stmt, 3));
        out_foodbatch->is_perishable = sqlite3_column_int(stmt, 4);
        strcpy(out_foodbatch->arrival_date, (const char *)sqlite3_column_text(stmt, 5));
        strcpy(out_foodbatch->expiration_date, (const char *)sqlite3_column_text(stmt, 6));
        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No FoodBatch found with BatchId: %d\n", batch_id);
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

bool foodbatch_db_check_batchid_exists(database *db, int batch_id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM FoodBatch WHERE BatchId = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, batch_id);

    bool exists = false;

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        exists = true; // A row was found with the given BatchID
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

int foodbatch_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM FoodBatch;";

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

int foodbatch_db_get_all_format(database *db, char *buffer, size_t buffer_size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    if (!buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return -1;
    }

    const char *sql = "SELECT * FROM FoodBatch;";
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
        "+-----------------------------------------------------------------------------------------+\n"
        "| ID  | Name             | Quantity | Unit  | Perishable | Arrival date | Expiration date |\n"
        "+-----+------------------+----------+-------+------------+--------------+-----------------+\n";

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
        fprintf(stderr, "Header truncated\n");
        return -1;
    }

    // Process each row
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int batch_id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        float quantity = (float)sqlite3_column_double(stmt, 2);
        const char *unit = (const char *)sqlite3_column_text(stmt, 3);
        int is_perishable = sqlite3_column_int(stmt, 4);
        const char *arrival_date = (const char *)sqlite3_column_text(stmt, 5);
        const char *expiration_date = (const char *)sqlite3_column_text(stmt, 6);

        // Format the row
        char row[1024];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-16s | %-8.2f | %-5s | %-10s | %-12s | %-15s |\n",
            batch_id,
            name,
            quantity,
            unit,
            (is_perishable == 0 ? "False" : "True"),
            arrival_date,
            expiration_date
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
            fprintf(stderr, "Row truncated\n");
            return -1;
        }

        // Add separator line
        const char *separator =
            "+-----+------------------+----------+-------+------------+--------------+-----------------+\n";

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
            fprintf(stderr, "Separator truncated\n");
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

char *foodbatch_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT * FROM FoodBatch;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

    //Header requires 277 (counting null terminator if no row) and each row will need at max 687

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
        "+-----------------------------------------------------------------------------------------+\n"
        "| ID  | Name             | Quantity | Unit  | Perishable | Arrival date | Expiration date |\n"
        "+-----+------------------+----------+-------+------------+--------------+-----------------+\n";


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
        int batch_id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        float quantity = (float)sqlite3_column_double(stmt, 2);
        const char *unit = (const char *)sqlite3_column_text(stmt, 3);
        int is_perishable = sqlite3_column_int(stmt, 4);
        const char *arrival_date = (const char *)sqlite3_column_text(stmt, 5);
        const char *expiration_date = (const char *)sqlite3_column_text(stmt, 6);

        // Format the row
        char row[1024];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-16s | %-8.2f | %-5s | %-10s | %-12s | %-15s |\n",
            batch_id,
            name,
            quantity,
            unit,
            (is_perishable == 0 ? "False" : "True"),
            arrival_date,
            expiration_date
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
            "+-----+------------------+----------+-------+------------+--------------+-----------------+\n";

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

int foodbatch_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM FoodBatch;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf("+-----------------------------------------------------------------------------------------+\n");
    printf("| ID  | Name             | Quantity | Unit  | Perishable | Arrival date | Expiration date |\n");
    printf("+-----+------------------+----------+-------+------------+--------------+-----------------+\n");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int batch_id = sqlite3_column_int(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        float quantity = (float)sqlite3_column_double(stmt, 2);
        const char *unit = (const char *)sqlite3_column_text(stmt, 3);
        int is_perishable = sqlite3_column_int(stmt, 4);
        const char *arrival_date = (const char *)sqlite3_column_text(stmt, 5);
        const char *expiration_date = (const char *)sqlite3_column_text(stmt, 6);

        printf(
            "| %-3d | %-16s | %-8.2f | %-5s | %-10s | %-12s | %-15s |\n",
            batch_id,
            name,
            quantity,
            unit,
            (is_perishable == 0 ? "False" : "True"),
            arrival_date,
            expiration_date
        );
        printf("+-----+------------------+----------+-------+------------+--------------+-----------------+\n");
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

/**
 * @file tasks_db.c
 * @brief Tasks database operations implementation
 */

#include "db/tasks_db.h"

#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tasks_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Tasks ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Title TEXT NOT NULL,"                      // Title of the task
        "Description TEXT NOT NULL,"                // Task summary (e.g., “restock diapers”)
        "DueDate TEXT,"                             // ISO 8601 date, e.g., "2024-07-01"
        "Priority INTEGER DEFAULT 0,"               // e.g., 0:low, 1:normal, 2:high
        "Status INTEGER DEFAULT 0,"                 // e.g., 0:pending, 1:in_progress, 2:done, 3:cancelled
        "AssignedTo TEXT,"                          // username or identifier (nullable, unassigned if NULL)
        "CreatedAt TEXT DEFAULT CURRENT_TIMESTAMP," // Creation timestamp
        "CompletedAt TEXT);";                       // When marked ‘done’. NULL if not yet done.

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Tasks table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}

int tasks_db_upsert(
    database *db,
    int id,
    const char *title,
    const char *description,
    const char *due_date,
    enum task_priority priority,
    enum task_status status,
    const char *assigned_to,
    const char *completed_at
) {
    if (!db_is_init(db)) {
        return SQLITE_ERROR;
    }

    sqlite3_stmt *stmt;
    int rc;

    if (id > 0) { // Update
        if (!tasks_db_check_exists(db, id)) {
            fprintf(stderr, "Tasks task with id %d does not exists.\n", id);
            return SQLITE_NOTFOUND;
        }

        const char *sql =
            "UPDATE Tasks SET Description = ?, DueDate = ?, Priority = ?, Status = ?, AssignedTo = ?, CompletedAt = ? WHERE ID = ?";

        rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            fprintf(
                stderr,
                "Not possible to get prepare statement: %s\non function %s, line %d\n",
                sqlite3_errmsg(db->db),
                __func__,
                __LINE__
            );
            return rc;
        }

        struct task tsk = { 0 };
        rc = tasks_db_get(db, id, &tsk);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            fprintf(stderr, "Could not fetch task for update.\n");
            return rc;
        }

        // Decide which fields to use for update based on inputs
        const char *desc_updt = (description && description[0] != '\0') ? description : tsk.description;
        const char *due_date_updt = (due_date && due_date[0] != '\0') ? due_date : tsk.due_date;
        enum task_priority priority_updt;
        if (priority != tsk.priority) {
            priority_updt = priority;
        } else {
            priority_updt = tsk.priority;
        }

        enum task_status status_updt;
        if (status != tsk.status) {
            status_updt = status;
        } else {
            status_updt = tsk.status;
        }

        const char *assigned_to_updt = (assigned_to && assigned_to[0] != '\0') ? assigned_to : tsk.assigned_to;

        // If we are changing status FROM TSK_DONE to NOT TSK_DONE, set CompletedAt to NULL.
        const char *completed_at_updt = NULL;
        if (tsk.status == TSK_DONE && status_updt != TSK_DONE) {
            completed_at_updt = NULL; // Will bind as null for reset
        } else if (completed_at && completed_at[0] != '\0') {
            completed_at_updt = completed_at;
        } else {
            completed_at_updt = tsk.completed_at;
        }

        sqlite3_bind_text(stmt, 1, desc_updt, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, due_date_updt, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, priority_updt);
        sqlite3_bind_int(stmt, 4, status_updt);

        if (assigned_to_updt) {
            sqlite3_bind_text(stmt, 5, assigned_to_updt, -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_null(stmt, 5);
        }

        if (completed_at_updt) {
            sqlite3_bind_text(stmt, 6, completed_at_updt, -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_bind_null(stmt, 6);
        }

        sqlite3_bind_int(stmt, 7, id);
    } else { // Insert
        const char *sql =
            "INSERT INTO Tasks (Title, Description, DueDate, Priority, Status, AssignedTo, CompletedAt) VALUES (?, ?, ?, ?, ?, ?, ?)";

        rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            return rc;
        }

        sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, description, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, due_date, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, priority);
        sqlite3_bind_int(stmt, 5, status);

        if (assigned_to) {
            sqlite3_bind_text(stmt, 6, assigned_to, -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 6);
        }

        if (completed_at) {
            sqlite3_bind_text(stmt, 7, completed_at, -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 7);
        }
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

bool tasks_db_check_exists(database *db, int id) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Tasks WHERE ID=?;";

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

int tasks_db_delete_entry_status(database *db, enum task_status status) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Tasks WHERE Status=?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, status);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);

    // Finalize the statement
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    if (sqlite3_changes(db->db) <= 0) {
        fprintf(stderr, "No entries where found with the given status.\n");
        return SQLITE_NOTFOUND;
    }

    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int tasks_db_get(database *db, int id, struct task *out_task) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT ID, Title, Description, DueDate, Priority, Status, AssignedTo, CreatedAt, CompletedAt FROM Tasks WHERE ID=?;";

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
        out_task->id = sqlite3_column_int(stmt, 0);
        strcpy(out_task->title, (const char *)sqlite3_column_text(stmt, 1));
        strcpy(out_task->description, (const char *)sqlite3_column_text(stmt, 2));
        strcpy(out_task->due_date, (const char *)sqlite3_column_text(stmt, 3));
        out_task->priority = sqlite3_column_int(stmt, 4);
        out_task->status = sqlite3_column_int(stmt, 5);
        strcpy(out_task->assigned_to, (const char *)sqlite3_column_text(stmt, 6));
        strcpy(out_task->created_at, (const char *)sqlite3_column_text(stmt, 7));
        const char *completed_at_val = (const char *)sqlite3_column_text(stmt, 8);
        if (completed_at_val) {
            strcpy(out_task->completed_at, (const char *)sqlite3_column_text(stmt, 8));
        } else {
            out_task->completed_at[0] = '\0';
        }

        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No Task found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int tasks_db_get_status(database *db, int id, enum task_status *out_status) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT Status FROM Tasks WHERE ID=?;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        *out_status = sqlite3_column_int(stmt, 0);
        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No Task found with the given data.\n");
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int tasks_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Tasks;";

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

int tasks_db_get_all_format(database *db, char *buffer, size_t buffer_size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    if (!buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return -1;
    }

    const char *sql = "SELECT * FROM Tasks;";
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
        "+-----------------------------------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Title            | Description                      | Due Date    | Priority | Status      | Assigned To      | Created At          | Completed At        |\n"
        "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n";

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
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        const char *description = (const char *)sqlite3_column_text(stmt, 2);
        const char *due_date = (const char *)sqlite3_column_text(stmt, 3);
        enum task_priority priority = sqlite3_column_int(stmt, 4);
        enum task_status status = sqlite3_column_int(stmt, 5);
        const char *assigned_to = (const char *)sqlite3_column_text(stmt, 6);
        const char *created_at = (const char *)sqlite3_column_text(stmt, 7);
        const char *completed_at = (const char *)sqlite3_column_text(stmt, 8);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-16s | %-32s | %-11s | %-8s | %-11s | %-16s | %-19s | %-19s |\n",
            id,
            title,
            description,
            due_date,
            task_priority_str[priority],
            task_status_str[status],
            assigned_to,
            created_at,
            completed_at
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
            "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n";

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

char *tasks_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT * FROM Tasks;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

    // Header will always needs 492 (493 (\0) if no other row) bytes and each row + separator will need at max 1029 (Last row 1030 for the null terminator) with the current table and format

    // Initial buffer
    size_t buffer_size = 4092;
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
        "+-----------------------------------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Title            | Description                      | Due Date    | Priority | Status      | Assigned To      | Created At          | Completed At        |\n"
        "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n";

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
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        const char *description = (const char *)sqlite3_column_text(stmt, 2);
        const char *due_date = (const char *)sqlite3_column_text(stmt, 3);
        enum task_priority priority = sqlite3_column_int(stmt, 4);
        enum task_status status = sqlite3_column_int(stmt, 5);
        const char *assigned_to = (const char *)sqlite3_column_text(stmt, 6);
        const char *created_at = (const char *)sqlite3_column_text(stmt, 7);
        const char *completed_at = (const char *)sqlite3_column_text(stmt, 8);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-3d | %-16s | %-32s | %-11s | %-8s | %-11s | %-16s | %-19s | %-19s |\n",
            id,
            title,
            description,
            due_date,
            task_priority_str[priority],
            task_status_str[status],
            assigned_to,
            created_at,
            completed_at
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
            "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n";

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

int tasks_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM Tasks;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf(
        "+-----------------------------------------------------------------------------------------------------------------------------------------------------------------+\n"
        "| ID  | Title            | Description                      | Due Date    | Priority | Status      | Assigned To      | Created At          | Completed At        |\n"
        "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        const char *description = (const char *)sqlite3_column_text(stmt, 2);
        const char *due_date = (const char *)sqlite3_column_text(stmt, 3);
        enum task_priority priority = sqlite3_column_int(stmt, 4);
        enum task_status status = sqlite3_column_int(stmt, 5);
        const char *assigned_to = (const char *)sqlite3_column_text(stmt, 6);
        const char *created_at = (const char *)sqlite3_column_text(stmt, 7);
        const char *completed_at = (const char *)sqlite3_column_text(stmt, 8);

        printf(
            "| %-3d | %-16s | %-32s | %-11s | %-8s | %-11s | %-16s | %-19s | %-19s |\n",
            id,
            title,
            description,
            due_date,
            task_priority_str[priority],
            task_status_str[status],
            assigned_to,
            created_at,
            completed_at
        );
        printf(
            "+-----+------------------+----------------------------------+-------------+----------+-------------+------------------+---------------------+---------------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

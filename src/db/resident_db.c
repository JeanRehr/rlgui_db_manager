/**
 * @file resident_db.c
 * @brief Resident database operations implementation
 */
#include "db/resident_db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)

int resident_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Resident ("
        "CPF TEXT PRIMARY KEY,"
        "Name TEXT NOT NULL,"
        "Age INTEGER NOT NULL,"
        "HealthStatus TEXT,"
        "Needs TEXT,"
        "MedicalAssistance INTEGER NOT NULL,"
        "Gender INTEGER NOT NULL,"
        "EntryDate TEXT);";

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Resident table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    return SQLITE_OK;
}

int resident_db_insert(
    database *db,
    const char *cpf,
    const char *name,
    int age,
    const char *health_status,
    const char *needs,
    bool medical_assistance,
    int gender
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "INSERT INTO Resident (CPF, Name, Age, HealthStatus, Needs, MedicalAssistance, Gender, EntryDate) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Get current time for EntryDate
    time_t now;
    time(&now);

    struct tm curr_time = { 0 };

#ifdef _WIN32
    localtime_s(&curr_time, &now);
#else
    localtime_r(&now, &curr_time);
#endif

    char date_string[32]; // Increased buffer to supress warning
    snprintf(
        date_string,
        sizeof(date_string),
        "%04d-%02d-%02d",
        curr_time.tm_year + 1900,
        curr_time.tm_mon + 1,
        curr_time.tm_mday
    );

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, age);
    sqlite3_bind_text(stmt, 4, health_status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, needs, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, medical_assistance ? 1 : 0);
    sqlite3_bind_int(stmt, 7, gender);
    sqlite3_bind_text(stmt, 8, date_string, -1, SQLITE_STATIC);

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

    // Finalize statement
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int resident_db_update(
    database *db,
    const char *cpf,
    const char *name_input,
    int age_input,
    const char *health_status_input,
    const char *needs_input,
    int medical_assistance_input,
    int gender_input
) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    struct resident currentResident = { 0 };
    int rc = resident_db_get_by_cpf(db, cpf, &currentResident);

    if (rc != SQLITE_OK) {
        fprintf(
            stderr,
            "Not possible to get resident by cpf: %s on function %s, line %d: %s\n",
            cpf,
            __func__,
            __LINE__,
            sqlite3_errmsg(db->db)
        );
        return rc;
    }

    // Decide which fields to use for update based on inputs
    const char *name = (name_input[0] != '\0') ? name_input : currentResident.name;
    int age = (age_input > 0) ? age_input : currentResident.age;
    const char *health_status = (health_status_input[0] != '\0') ? health_status_input : currentResident.health_status;
    const char *needs = (needs_input[0] != '\0') ? needs_input : currentResident.needs;
    int medical_assistance =
        (medical_assistance_input > 0) ? medical_assistance_input : currentResident.medical_assistance;
    int gender = (gender_input >= 0) ? gender_input : (int)currentResident.gender;

    const char *sql =
        "UPDATE Resident SET Name = ?, Age = ?, HealthStatus = ?, Needs = ?, MedicalAssistance = ?, Gender "
        "= ? WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_text(stmt, 3, health_status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, needs, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, medical_assistance ? 1 : 0);
    sqlite3_bind_int(stmt, 6, gender);
    sqlite3_bind_text(stmt, 7, cpf, -1, SQLITE_STATIC);

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

int resident_db_delete_by_cpf(database *db, const char *cpf) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!resident_db_check_cpf_exists(db, cpf)) {
        fprintf(stderr, "CPF not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    // Prepare the SQL delete statement
    const char *sql = "DELETE FROM Resident WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Bind the CPF parameter
    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    // Execute the DELETE statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

bool resident_db_check_cpf_exists(database *db, const char *cpf) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Resident WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    bool exists = false;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        exists = true; // A row was found with the given CPF
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

int resident_db_get_by_cpf(database *db, const char *cpf, struct resident *resident) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT CPF, Name, Age, HealthStatus, Needs, MedicalAssistance, Gender, EntryDate FROM Resident WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strcpy(resident->cpf, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(resident->name, (const char *)sqlite3_column_text(stmt, 1));
        resident->age = sqlite3_column_int(stmt, 2);
        strcpy(resident->health_status, (const char *)sqlite3_column_text(stmt, 3));
        strcpy(resident->needs, (const char *)sqlite3_column_text(stmt, 4));
        resident->medical_assistance = sqlite3_column_int(stmt, 5);
        resident->gender = sqlite3_column_int(stmt, 6);
        strcpy(resident->entry_date, (const char *)sqlite3_column_text(stmt, 7));
        rc = SQLITE_OK; // Found and read successfully
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No resident found with CPF: %s\n", cpf);
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int resident_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Resident;";

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

int resident_db_get_all_format(database *db, char *buffer, size_t buffer_size) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    if (!buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid buffer provided.\n");
        return -1;
    }

    const char *sql = "SELECT * FROM Resident;";
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
        "+-----------------------------------------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------+\n"
        "| CPF         | Name                                       | Age | HealthStatus                             "
        "  | Needs                                      | Medical Assistance | Gender | Entry Date |\n"
        "+-------------+--------------------------------------------+-----+-------------------------------------"
        "-----"
        "--+--------------------------------------------+--------------------+--------+------------+\n";

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
        const char *cpf = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *health_status = (const char *)sqlite3_column_text(stmt, 3);
        const char *needs = (const char *)sqlite3_column_text(stmt, 4);
        int medical_assistance = sqlite3_column_int(stmt, 5);
        int gender = sqlite3_column_int(stmt, 6);
        const char *entry_date = (const char *)sqlite3_column_text(stmt, 7);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-11s | %-42s | %-3d | %-42s | %-42s | %-18s | %-6s | %-10s |\n",
            cpf,
            name,
            age,
            health_status,
            needs,
            (medical_assistance == 0 ? "False" : "True"),
            (gender == 0 ? "Other" : (gender == 1 ? "Male" : "Female")),
            entry_date
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
            "+-------------+--------------------------------------------+-----+-------------------------------------"
            "-----"
            "--+--------------------------------------------+--------------------+--------+------------+\n";

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

char *resident_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT * FROM Resident;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

    // Header will always needs 601 bytes and each row + separator will need at max 1040 with the current table and format

    // Initial buffer
    size_t buffer_size = 1024;
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
        "+-------------------------------------------------------------------------------------------------------------"
        "----------------------------------------------------------------------------------------+\n"
        "| CPF         | Name                                       | Age | HealthStatus                               "
        "| Needs                                      | Medical Assistance | Gender | Entry Date |"
        "\n+-------------+--------------------------------------------+-----+------------------------------------------"
        "--+--------------------------------------------+--------------------+--------+------------+\n";

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
        const char *cpf = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *health_status = (const char *)sqlite3_column_text(stmt, 3);
        const char *needs = (const char *)sqlite3_column_text(stmt, 4);
        int medical_assistance = sqlite3_column_int(stmt, 5);
        int gender = sqlite3_column_int(stmt, 6);
        const char *entry_date = (const char *)sqlite3_column_text(stmt, 7);

        // Format the row
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-11s | %-42s | %-3d | %-42s | %-42s | %-18s | %-6s | %-10s |\n",
            cpf,
            name,
            age,
            health_status,
            needs,
            (medical_assistance == 0 ? "False" : "True"),
            (gender == 0 ? "Other" : (gender == 1 ? "Male" : "Female")),
            entry_date
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
            "+-------------+--------------------------------------------+-----+-------------------------------------"
            "-------+--------------------------------------------+--------------------+--------+------------+\n";
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

int resident_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT * FROM Resident;";

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    printf(
        "+-----------------------------------------------------------------------------------------------------------"
        "------------------------------------------------------------------------------------------+\n"
        "| CPF         | Name                                       | Age | HealthStatus                             "
        "  | Needs                                      | Medical Assistance | Gender | Entry Date |\n"
    );
    printf(
        "+-------------+--------------------------------------------+-----+------------------------------------------"
        "--+--------------------------------------------+--------------------+--------+------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *cpf = (const char *)sqlite3_column_text(stmt, 0);
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *health_status = (const char *)sqlite3_column_text(stmt, 3);
        const char *needs = (const char *)sqlite3_column_text(stmt, 4);
        int medical_assistance = sqlite3_column_int(stmt, 5);
        int gender = sqlite3_column_int(stmt, 6);
        const char *entry_date = (const char *)sqlite3_column_text(stmt, 7);

        printf(
            "| %-11s | %-42s | %-3d | %-42s | %-42s | %-18s | %-6s | %-10s |\n",
            cpf,
            name,
            age,
            health_status,
            needs,
            (medical_assistance == 0 ? "False" : "true"),
            (gender == 0 ? "Other" : (gender == 1 ? "Male" : "Female")),
            entry_date
        );
        printf(
            "+-------------+--------------------------------------------+-----+-------------------------------------"
            "-----"
            "--+--------------------------------------------+--------------------+--------+------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

/**
 * @file user_db.c
 * @brief User database operations implementation
 */
#include "db/user_db.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)

#include "utils_hash.h"

int user_db_create_table(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS Users ("
        "Username TEXT PRIMARY KEY NOT NULL,"
        "PasswordHash TEXT,"
        "Salt TEXT,"
        "PhoneNumber TEXT,"
        "CPF TEXT UNIQUE NOT NULL,"
        "IsAdmin INTEGER NOT NULL DEFAULT 0,"
        "ResetPassword INTEGER NOT NULL DEFAULT 1,"
        "CreatedAt INTEGER NOT NULL,"
        "LastLogin INTEGER);";

    char *errMsg = 0;
    int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error on init Users Table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    if (!user_db_check_exists(db, "admin")) {
        user_db_create_admin(db);
    }

    return SQLITE_OK;
}

int user_db_create_user(database *db, const char *username, const char *cpf, const char *phone_number, bool is_admin) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (user_db_check_exists(db, username)) {
        fprintf(stderr, "Username already exists.\n");
        return SQLITE_CONSTRAINT;
    }

    if (user_db_check_cpf_exists(db, cpf)) {
        fprintf(stderr, "CPF already exists.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql =
        "INSERT INTO Users (Username, CPF, PhoneNumber, IsAdmin, ResetPassword, CreatedAt) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    time_t now = time(NULL);

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cpf, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, phone_number, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, is_admin ? 1 : 0);
    sqlite3_bind_int(stmt, 5, 1);
    sqlite3_bind_int64(stmt, 6, now);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int user_db_create_admin(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (user_db_check_exists(db, "admin")) {
        fprintf(stderr, "Admin already exists.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *password = "admin";
    const char *username = "admin";
    bool is_admin = true;
    bool reset_password = false;

    // Generate salt and hash the password
    char salt[SALT_LEN + 1] = { 0 };
    char hash[PASSWORD_HASH_LEN + 1] = { 0 };
    generate_salt(salt, SALT_LEN);
    hash_password(password, salt, hash);

    const char *sql =
        "INSERT INTO Users (Username, PasswordHash, Salt, CPF, PhoneNumber, IsAdmin, ResetPassword, CreatedAt) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // Get current time.
    time_t now = time(NULL);

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, "99999999999", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, "5551900000000", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, is_admin ? 1 : 0);
    sqlite3_bind_int(stmt, 7, reset_password ? 1 : 0);
    sqlite3_bind_int64(stmt, 8, now);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

enum auth_result user_db_authenticate(database *db, const char *username, const char *password) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return AUTH_FAILURE;
    }

    struct user user = { 0 };
    if (user_db_get_by_username(db, username, &user) != SQLITE_OK) {
        printf("User '%s' not found in database\n", username);
        return AUTH_FAILURE; // user was not found
    }

    // Check if password needs to be reset
    if (user.reset_password) {
        return AUTH_NEED_PASSWORD_RESET;
    }

    // Verify password
    char computed_hash[PASSWORD_HASH_LEN + 1] = { 0 };
    hash_password(password, user.salt, computed_hash);

    if (strcmp(computed_hash, user.password_hash) != 0) {
        return AUTH_FAILURE;
    }

    // Update last login time
    const char *sql = "UPDATE Users SET LastLogin = ? WHERE Username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return AUTH_SUCCESS; // Authentication succeeded even if we can't update last login
    }

    time_t now = time(NULL);
    sqlite3_bind_int64(stmt, 1, now);
    sqlite3_bind_text(stmt, 2, user.username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update last login time: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return AUTH_SUCCESS;
}

int user_db_delete(database *db, const char *username) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!user_db_check_exists(db, username)) {
        fprintf(stderr, "Username not found in the dabatase.\n");
        return SQLITE_NOTFOUND;
    }

    if (strcmp(username, "admin") == 0) {
        fprintf(stderr, "Default admin cannot be deleted.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql = "DELETE FROM Users WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete user: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int user_db_update_password(database *db, const char *username, const char *new_password) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!user_db_check_exists(db, username)) {
        fprintf(stderr, "User does not exist.\n");
        return SQLITE_NOTFOUND;
    }

    // Generate new salt and hash
    char salt[SALT_LEN + 1] = { 0 };
    char hash[PASSWORD_HASH_LEN + 1] = { 0 };
    generate_salt(salt, SALT_LEN);
    hash_password(new_password, salt, hash);

    const char *sql = "UPDATE Users SET PasswordHash = ?, Salt = ?, ResetPassword = 0 WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, salt, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update password: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int user_db_update_admin_status(database *db, const char *username, bool is_admin) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!user_db_check_exists(db, username)) {
        fprintf(stderr, "User does not exist.\n");
        return SQLITE_NOTFOUND;
    }

    if (strcmp(username, "admin") == 0) {
        fprintf(stderr, "Can't change default admin status.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql = "UPDATE Users SET IsAdmin = ? WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, is_admin ? 1 : 0);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update admin status: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

bool user_db_check_cpf_exists(database *db, const char *cpf) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Users WHERE CPF = ?;";

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
        exists = true;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return exists;
}

bool user_db_check_exists(database *db, const char *username) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    const char *sql = "SELECT 1 FROM Users WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    bool exists = false;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        exists = true;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return exists;
}

int user_db_get_by_username(database *db, const char *username, struct user *user_out) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql =
        "SELECT Username, PasswordHash, Salt, CPF, PhoneNumber, IsAdmin, ResetPassword, CreatedAt, LastLogin "
        "FROM Users WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *text;

        // Username (column 0)
        text = sqlite3_column_text(stmt, 0);
        strncpy(user_out->username, text ? (const char *)text : "", MAX_INPUT);

        // PasswordHash (column 1)
        text = sqlite3_column_text(stmt, 1);
        strncpy(user_out->password_hash, text ? (const char *)text : "", PASSWORD_HASH_LEN);

        // Salt (column 2)
        text = sqlite3_column_text(stmt, 2);
        strncpy(user_out->salt, text ? (const char *)text : "", SALT_LEN);

        // CPF (column 3)
        text = sqlite3_column_text(stmt, 3);
        strncpy(user_out->cpf, text ? (const char *)text : "", MAX_CPF_LENGTH);

        // PhoneNumber (column 4)
        text = sqlite3_column_text(stmt, 4);
        strncpy(user_out->phone_number, text ? (const char *)text : "", MAX_PHONE_NUMBER_LEN);

        // IsAdmin (column 5)
        user_out->is_admin = sqlite3_column_int(stmt, 5) == 1;

        // ResetPassword (column 6)
        user_out->reset_password = sqlite3_column_int(stmt, 6) == 1;

        // CreatedAt (column 7)
        user_out->created_at = sqlite3_column_int64(stmt, 7);

        // LastLogin (column 8) - can be NULL
        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            user_out->last_login = sqlite3_column_int64(stmt, 8);
        } else {
            user_out->last_login = 0;
        }

        rc = SQLITE_OK;
    } else if (rc == SQLITE_DONE) {
        rc = SQLITE_NOTFOUND;
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int user_db_change_username(database *db, const char *old_username, const char *new_username) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (strcmp(old_username, "admin") == 0) {
        fprintf(stderr, "Can't change default admin username.\n");
        return SQLITE_CONSTRAINT;
    }

    // Check if the old username exists
    if (!user_db_check_exists(db, old_username)) {
        fprintf(stderr, "Old username not found in the database.\n");
        return SQLITE_NOTFOUND;
    }

    // Check if the new username already exists
    if (user_db_check_exists(db, new_username)) {
        fprintf(stderr, "New username already exists in the database.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *update_sql = "UPDATE Users SET Username = ? WHERE Username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }
    sqlite3_bind_text(stmt, 1, new_username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, old_username, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update username: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    return SQLITE_OK;
}

bool user_db_check_admin_status(database *db, const char *username) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return false;
    }

    if (!user_db_check_exists(db, username)) {
        fprintf(stderr, "Username does not exists.\n");
        return false;
    }

    const char *sql = "SELECT IsAdmin FROM Users WHERE Username = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    bool is_admin = false;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        is_admin = sqlite3_column_int(stmt, 0) == 1;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return is_admin;
}

int user_db_set_reset_password(database *db, const char *username) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (!user_db_check_exists(db, username)) {
        fprintf(stderr, "User does not exist.\n");
        return SQLITE_NOTFOUND;
    }

    const char *sql = "UPDATE Users SET ResetPassword = 1 WHERE Username = ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to set reset password: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int user_db_get_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    const char *sql = "SELECT COUNT(*) FROM Users;";

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

char *user_db_get_all_format_old(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return NULL;
    }

    const char *sql = "SELECT * FROM Users;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return NULL;
    }

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
        "+------------------------------------------------------------------------------------------------------+\n"
        "| Username                 | CPF         | Phone Number  | Admin | Created At       | Last Login       |\n"
        "+--------------------------+-------------+---------------+-------+------------------+------------------+\n";

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
        const char *username = (const char *)sqlite3_column_text(stmt, 0);
        const char *cpf = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone_number = (const char *)sqlite3_column_text(stmt, 2);
        bool is_admin = sqlite3_column_int(stmt, 3) == 1;
        time_t created_at = sqlite3_column_int64(stmt, 4);
        time_t last_login = sqlite3_column_int64(stmt, 5);

        char created_at_str[32];
        char last_login_str[32];
        strftime(created_at_str, sizeof(created_at_str), "%Y-%m-%d %H:%M", localtime(&created_at));
        if (last_login > 0) {
            strftime(last_login_str, sizeof(last_login_str), "%Y-%m-%d %H:%M", localtime(&last_login));
        } else {
            strcpy(last_login_str, "Never");
        }
        
        char row[2048];
        snprintf(
            row,
            sizeof(row),
            "| %-24s | %-11s | %-13s | %-5s | %-16s | %-16s |\n",
            username,
            cpf,
            phone_number,
            is_admin ? "Yes" : "No",
            created_at_str,
            last_login_str
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
            "+--------------------------+-------------+---------------+-------+------------------+------------------+\n";
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

int user_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT Username, CPF, PhoneNumber, IsAdmin, CreatedAt, LastLogin FROM Users;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }
    printf(
        "+------------------------------------------------------------------------------------------------------+\n"
    );
    printf(
        "| Username                 | CPF         | Phone Number  | Admin | Created At       | Last Login       |\n"
    );
    printf(
        "+--------------------------+-------------+---------------+-------+------------------+------------------+\n"
    );

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *username = (const char *)sqlite3_column_text(stmt, 0);
        const char *cpf = (const char *)sqlite3_column_text(stmt, 1);
        const char *phone_number = (const char *)sqlite3_column_text(stmt, 2);
        bool is_admin = sqlite3_column_int(stmt, 3) == 1;
        time_t created_at = sqlite3_column_int64(stmt, 4);
        time_t last_login = sqlite3_column_int64(stmt, 5);

        char created_at_str[32];
        char last_login_str[32];
        strftime(created_at_str, sizeof(created_at_str), "%Y-%m-%d %H:%M", localtime(&created_at));
        if (last_login > 0) {
            strftime(last_login_str, sizeof(last_login_str), "%Y-%m-%d %H:%M", localtime(&last_login));
        } else {
            strcpy(last_login_str, "Never");
        }

        printf(
            "| %-24s | %-11s | %-13s | %-5s | %-16s | %-16s |\n",
            username,
            cpf,
            phone_number,
            is_admin ? "Yes" : "No",
            created_at_str,
            last_login_str
        );
        printf(
            "+--------------------------+-------------+---------------+-------+------------------+------------------+\n"
        );
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

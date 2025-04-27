#include <external/sqlite3/sqlite3.h>

#include "db/user_db.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

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

int user_db_create_user(database *db, const char *username, bool is_admin, bool reset_password) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    if (user_db_check_exists(db, username)) {
        fprintf(stderr, "Username already exists.\n");
        return SQLITE_CONSTRAINT;
    }

    const char *sql =
        "INSERT INTO Users (Username, IsAdmin, ResetPassword, CreatedAt) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    time_t now = time(NULL);

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, is_admin ? 1 : 0);
    sqlite3_bind_int(stmt, 3, reset_password ? 1 : 0);
    sqlite3_bind_int64(stmt, 4, now);

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
        "INSERT INTO Users (Username, PasswordHash, Salt, IsAdmin, ResetPassword, CreatedAt) "
        "VALUES (?, ?, ?, ?, ?, ?);";

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
    sqlite3_bind_int(stmt, 4, is_admin ? 1 : 0);
    sqlite3_bind_int(stmt, 5, reset_password ? 1 : 0);
    sqlite3_bind_int64(stmt, 6, now);

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
        return true; // Authentication succeeded even if we can't update last login
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
        "SELECT Username, PasswordHash, Salt, IsAdmin, ResetPassword, CreatedAt, LastLogin "
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
        // Safely handle text fields that might be NULL
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

        // IsAdmin (column 3)
        user_out->is_admin = sqlite3_column_int(stmt, 3) == 1;
        
        // ResetPassword (column 4)
        user_out->reset_password = sqlite3_column_int(stmt, 4) == 1;
        
        // CreatedAt (column 5)
        user_out->created_at = sqlite3_column_int64(stmt, 5);

        // LastLogin (column 6) - can be NULL
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            user_out->last_login = sqlite3_column_int64(stmt, 6);
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

    // Begin transaction since we're modifying the primary key
    int rc = sqlite3_exec(db->db, "BEGIN TRANSACTION;", 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to begin transaction: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    // First get all user data
    struct user user_data = {0};
    rc = user_db_get_by_username(db, old_username, &user_data);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db->db, "ROLLBACK;", 0, 0, 0);
        return rc;
    }

    // Create a new user with the new username and same data
    const char *insert_sql =
        "INSERT INTO Users (Username, PasswordHash, Salt, IsAdmin, ResetPassword, CreatedAt, LastLogin) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *insert_stmt;
    rc = sqlite3_prepare_v2(db->db, insert_sql, -1, &insert_stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare insert statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_exec(db->db, "ROLLBACK;", 0, 0, 0);
        return rc;
    }

    sqlite3_bind_text(insert_stmt, 1, new_username, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, user_data.password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 3, user_data.salt, -1, SQLITE_STATIC);
    sqlite3_bind_int(insert_stmt, 4, user_data.is_admin ? 1 : 0);
    sqlite3_bind_int(insert_stmt, 5, user_data.reset_password ? 1 : 0);
    sqlite3_bind_int64(insert_stmt, 6, user_data.created_at);
    sqlite3_bind_int64(insert_stmt, 7, user_data.last_login);

    rc = sqlite3_step(insert_stmt);
    sqlite3_finalize(insert_stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to create new username entry: %s\n", sqlite3_errmsg(db->db));
        sqlite3_exec(db->db, "ROLLBACK;", 0, 0, 0);
        return rc;
    }

    // Delete the old user entry
    const char *delete_sql = "DELETE FROM Users WHERE Username = ?;";
    sqlite3_stmt *delete_stmt;
    rc = sqlite3_prepare_v2(db->db, delete_sql, -1, &delete_stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
        sqlite3_exec(db->db, "ROLLBACK;", 0, 0, 0);
        return rc;
    }

    sqlite3_bind_text(delete_stmt, 1, old_username, -1, SQLITE_STATIC);
    rc = sqlite3_step(delete_stmt);
    sqlite3_finalize(delete_stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to delete old username entry: %s\n", sqlite3_errmsg(db->db));
        sqlite3_exec(db->db, "ROLLBACK;", 0, 0, 0);
        return rc;
    }

    // Commit the transaction if everything succeeded
    rc = sqlite3_exec(db->db, "COMMIT;", 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to commit transaction: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    return SQLITE_OK;
}


int user_db_get_all(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return SQLITE_ERROR;
    }

    const char *sql = "SELECT Username, IsAdmin, CreatedAt, LastLogin FROM Users;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }
    printf("+------------------------------------------------------------------------+\n");
    printf("| Username                 | Admin | Created At       | Last Login       |\n");
    printf("+--------------------------+-------+------------------+------------------+\n");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *username = (const char *)sqlite3_column_text(stmt, 0);
        bool is_admin = sqlite3_column_int(stmt, 1) == 1;
        time_t created_at = sqlite3_column_int64(stmt, 2);
        time_t last_login = sqlite3_column_int64(stmt, 3);

        char created_at_str[20];
        char last_login_str[20];
        strftime(created_at_str, sizeof(created_at_str), "%Y-%m-%d %H:%M", localtime(&created_at));
        if (last_login > 0) {
            strftime(last_login_str, sizeof(last_login_str), "%Y-%m-%d %H:%M", localtime(&last_login));
        } else {
            strcpy(last_login_str, "Never");
        }

        printf(
            "| %-24s | %-5s | %-16s | %-16s |\n",
            username,
            is_admin ? "Yes" : "No",
            created_at_str,
            last_login_str
        );
        printf("+--------------------------+-------+------------------+------------------+\n");
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}
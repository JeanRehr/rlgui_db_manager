/**
 * @file user_db.h
 * @brief User Database Operations
 *
 * This header defines operations for managing user accounts in an SQLite database,
 * including authentication, password management, and user administration.
 */

#ifndef USER_DB_H
#define USER_DB_H

#include "db/db_manager.h"
#include "entities/user.h"

/**
 * @enum auth_result
 * @brief Possible results of authentication attempts
 */
enum auth_result {
    AUTH_FAILURE,            ///< Authentication failed
    AUTH_SUCCESS,            ///< Authentication succeeded
    AUTH_NEED_PASSWORD_RESET ///< Authentication requires password reset
};

/**
 * @brief Creates the Users table in the database
 *
 * Creates a new Users table if it doesn't already exist, including fields for
 * authentication, administration, and timestamps. Automatically creates an
 * admin account if one doesn't exist.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLite error code on failure
 * @warning Requires an initialized database connection
 */
int user_db_create_table(database *db);

/**
 * @brief Creates a new user account
 *
 * Adds a new user record with the specified username and admin status.
 * The account will be created with a temporary password that must be reset.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Unique username for the new account this is the primary key
 * @param[in] cpf Unique CPF for the new account
 * @param[in] phone_number Phone number for the new account, may be null
 * @param[in] is_admin Whether the user should have admin privileges
 *
 * @note The newly created user will always need a password reset
 *
 * @return SQLITE_OK on success, SQLITE_CONSTRAINT if username exists, or other SQLite error code
 */
int user_db_create_user(database *db, const char *username, const char *cpf, const char *phone_number, bool is_admin);

/**
 * @brief Creates the default admin account
 *
 * Creates a special admin account with default credentials if one doesn't exist.
 * This is automatically called during table creation if no admin exists.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, SQLITE_CONSTRAINT if admin exists, or other SQLite error code
 * @note The default admin password is "admin" and cannot be deleted
 */
int user_db_create_admin(database *db);

/**
 * @brief Authenticates a user
 *
 * Verifies user credentials and returns authentication status. Updates last login
 * time on successful authentication.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username to authenticate
 * @param[in] password Password to verify
 * @return AUTH_SUCCESS on success, AUTH_NEED_PASSWORD_RESET if password reset required,
 *         or AUTH_FAILURE on failure
 */
enum auth_result user_db_authenticate(database *db, const char *username, const char *password);

/**
 * @brief Deletes a user account
 *
 * Removes the specified user account from the database. The default admin
 * account cannot be deleted.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username of account to delete
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist,
 *         SQLITE_CONSTRAINT if trying to delete default admin, or other SQLite error code
 */
int user_db_delete(database *db, const char *username);

/**
 * @brief Updates a user's phone number
 *
 * Changes the phone_number for the specified user account
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username of account to update
 * @param[in] phone_number New phone number to set
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist, or other SQLite error code
 */
int user_db_update_phone_number(database *db, const char *username, const char *phone_number);

/**
 * @brief Updates a user's cpf
 *
 * Changes the cpf for the specified user account.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username of account to update
 * @param[in] cpf New CPF to set
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist, or other SQLite error code
 */
int user_db_update_cpf(database *db, const char *username, const char *cpf);

/**
 * @brief Updates a user's password
 *
 * Changes the password for the specified user account, generating a new salt
 * and clearing any password reset requirement.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username of account to update
 * @param[in] new_password New password to set
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist, or other SQLite error code
 */
int user_db_update_password(database *db, const char *username, const char *new_password);

/**
 * @brief Updates a user's admin status
 *
 * Changes the administrator privileges for the specified user account.
 * The default admin account's status cannot be changed.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username of account to modify
 * @param[in] is_admin New admin status (true = admin)
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist,
 *         SQLITE_CONSTRAINT if modifying default admin, or other SQLite error code
 */
int user_db_update_admin_status(database *db, const char *username, bool is_admin);

/**
 * @brief Checks if a cpf exists
 *
 * Verifies whether an account with the specified cpf exists.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] cpf CPF to check
 * @return true if cpf exists, false otherwise
 */
bool user_db_check_cpf_exists(database *db, const char *cpf);

/**
 * @brief Checks if a username exists
 *
 * Verifies whether an account with the specified username exists.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username to check
 * @return true if username exists, false otherwise
 */
bool user_db_check_exists(database *db, const char *username);

/**
 * @brief Retrieves user account details
 *
 * Fetches complete account information for the specified username.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username to retrieve
 * @param[out] user_out Pointer to structure where data will be stored
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist, or other SQLite error code
 */
int user_db_get_by_username(database *db, const char *username, struct user *user_out);

/**
 * @brief Changes a user's username
 *
 * Updates the primary key username for an account while preserving all other data.
 * The default admin account's username cannot be changed.
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] old_username Current username
 * @param[in] new_username Desired new username
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if old username doesn't exist,
 *         SQLITE_CONSTRAINT if new username exists or modifying admin, or other SQLite error code
 */
int user_db_update_username(database *db, const char *old_username, const char *new_username);

/**
 * @brief Check if the given username has admin status
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username to check
 * @return true if username has admin status, false otherwise or if the username does not exists
 */
bool user_db_check_admin_status(database *db, const char *username);

/**
 * @brief Set the given username to reset password
 *
 * @param[in] db Pointer to initialized database structure
 * @param[in] username Username to set
 * @return SQLITE_OK on success, SQLITE_NOTFOUND if user doesn't exist, or other SQLite error code
 */
int user_db_set_reset_password(database *db, const char *username);

/**
 * @brief Gets the count of registered users in the database
 *
 * @param db Pointer to initialized database structure
 * @return Total number of users on success, -1 on failure
 *
 * @note The user database will always have a default admin created
 */
int user_db_get_count(database *db);

/**
 * @brief Writes all user records as a formatted string into provided buffer
 *
 * Executes a database query and formats all user records into a human-readable
 * table structure with borders and aligned columns.
 *
 * @param db Pointer to initialized database connection
 * @param buffer Pointer to buffer where formatted string will be written
 * @param buffer_size Size of the provided buffer
 * @return int Number of bytes written (excluding null terminator), or -1 on failure
 *
 * @note Header will always needs 316 bytes and each row + separator (Considering max input is 256)
 *       will need at max 441 with the current table and format
 *       String format:
 * +------------------------------------------------------------------------------------------------------+
 * | Username                 | CPF         | Phone Number  | Admin | Created At       | Last Login       |
 * +--------------------------+-------------+---------------+-------+------------------+------------------+
 * | admin                    | 99999999999 | 5551900000000 | Yes   | 2000-01-31 00:00 | 2000-01-31 00:01 |
 * +--------------------------+-------------+---------------+-------+------------------+------------------+
 *
 * @warning Returns -1 if database is not initialized, on query failure, or if buffer is too small
 * @warning Buffer will be null-terminated if there's space, even on truncation
 *
 * Memory Management:
 * - Caller provides buffer and manages its memory
 * - Function never allocates memory
 *
 * Error Handling:
 * - Checks database connection state
 * - Validates SQL preparation
 * - Reports SQL execution errors
 * - Handles buffer overflow
 */
int user_db_get_all_format(database *db, char *buffer, size_t buffer_size);

/**
 * @brief Retrieves all user records as a formatted string
 *
 * Executes a database query and formats all user records into a human-readable
 * table structure with borders and aligned columns. The returned string is dynamically
 * allocated and must be freed by the caller.
 *
 * This was mostly used to calculate more or less how many bytes each row + header will need at max
 *
 * @param db Pointer to initialized database connection
 * @return char* Formatted table string containing all records, or NULL on failure
 *
 * @note Returned string format:
 * +------------------------------------------------------------------------------------------------------+
 * | Username                 | CPF         | Phone Number  | Admin | Created At       | Last Login       |
 * +--------------------------+-------------+---------------+-------+------------------+------------------+
 * | admin                    | 99999999999 | 5551900000000 | Yes   | 2025-05-22 00:15 | 2025-05-22 00:31 |
 * +--------------------------+-------------+---------------+-------+------------------+------------------+
 * 
 * @note HEADER (316 bytes) + default admin user + separator (106 bytes (\n+\0)) will allocate 526 bytes (counting null terminator)
 *       each subsequent user + separator will need at most 441 (counting null terminator) with the current table and format.
 *       Only the Header will need 316 bytes (counting null terminator)
 *
 * @warning The caller is responsible for freeing the returned string with free()
 * @warning Returns NULL if database is not initialized or on query failure
 * @warning This is not safe as it does not adhere to the memory encapsulation principle
 *
 * Memory Management:
 * - Allocates initial 4KB buffer
 * - Automatically grows buffer as needed
 * - Returns NULL on allocation failures
 *
 * Error Handling:
 * - Checks database connection state
 * - Validates SQL preparation
 * - Handles memory allocation failures
 * - Reports SQL execution errors
 *
 * @deprecated
 */
char *user_db_get_all_format_old(database *db);

/**
 * @brief Retrieves and displays all user accounts
 *
 * Fetches all user records and displays them in a formatted table showing
 * usernames, admin status, and timestamps.
 *
 * @param[in] db Pointer to initialized database structure
 * @return SQLITE_OK on success, or other SQLite error code
 * @note Output is printed directly to stdout in table format
 */
int user_db_get_all(database *db);

#endif // USER_DB_H

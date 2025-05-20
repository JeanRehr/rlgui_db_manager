/**
 * @file user.h
 * @brief User definition for use in database operations/code
 */
#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include <time.h>

#include "CONSTANTS.h"

/** Length constants for password hashing */
#define PASSWORD_HASH_LEN 64 ///< Length of SHA-256 hex string (2 chars per byte)
#define SALT_LEN 32          ///< Recommended length for cryptographic salts

/**
 * @struct user
 * @brief Represents a user account in the database
 */
struct user {
    char username[MAX_INPUT];                  ///< Unique username identifier
    char password_hash[PASSWORD_HASH_LEN + 1]; ///< Hashed password
    char salt[SALT_LEN + 1];                   ///< Password salt
    char cpf[MAX_CPF_LENGTH];                  ///< CPF (must be unique per user)
    char phone_number[MAX_PHONE_NUMBER_LEN];   ///< Contact data
    bool is_admin;                             ///< Administrator flag
    bool reset_password;                       ///< Password reset required flag
    time_t created_at;                         ///< Account creation timestamp
    time_t last_login;                         ///< Last login timestamp (0 if never)
};

#endif // USER_H

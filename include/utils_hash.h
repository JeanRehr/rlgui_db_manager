/**
 * @file utils_hash.h
 * @brief Password Hashing Utilities
 * 
 * Provides cryptographic functions for secure password handling including
 * salt generation and password hashing using OpenSSL's SHA-256.
 */

#ifndef UTILS_HASH_H
#define UTILS_HASH_H

#include <stddef.h>

/**
 * @brief Generates a cryptographically secure random salt
 * 
 * Creates a random salt value using OpenSSL's RAND_bytes() with a fallback
 * to standard rand() if cryptographic randomness is unavailable.
 * 
 * @param[out] salt Buffer to store the generated salt (must be at least len+1 bytes)
 * @param[in] len Length of salt to generate (typically SALT_LEN)
 * @warning Uses OpenSSL's RAND_bytes() which must be properly seeded
 * @note Always null-terminates the salt string
 */
void generate_salt(char *salt, size_t len);

/**
 * @brief Hashes a password with a given salt using SHA-256
 * 
 * Creates a cryptographic hash of the password+salt combination and outputs
 * a hex-encoded SHA-256 digest.
 * 
 * @param[in] password Plaintext password to hash
 * @param[in] salt Salt value to combine with password
 * @param[out] hash_out Buffer to store hex-encoded hash (must be at least 65 bytes)
 * 
 * @warning Never store plaintext passwords - only store the hashed output
 * @note Output is a 64-character hex string + null terminator
 * 
 * Example usage:
 * @code
 * char salt[SALT_LEN+1];
 * char hash[PASSWORD_HASH_LEN+1];
 * generate_salt(salt, SALT_LEN);
 * hash_password("mypassword", salt, hash);
 * @endcode
 */
void hash_password(const char *password, const char *salt, char *hash_out);

#endif // UTILS_HASH_H

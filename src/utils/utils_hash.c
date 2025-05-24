/**
 * @file utils_hash.c
 * @brief Utils hashing functions implementations
 */
#include "utils/utils_hash.h"

#ifdef _WIN32
    // Isolated to this file to not have name mangling of functions
    // Not actually using anything windows specific
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include <stdio.h>
#include <string.h>

#include <external/openssl/rand.h>
#include <external/openssl/sha.h>

void generate_salt(char *salt, size_t len) {
    if (RAND_bytes((unsigned char *)salt, len) != 1) {
        // Fallback to less secure method if RAND_bytes fails
        for (size_t i = 0; i < len; i++) {
            salt[i] = (char)(rand() % 256);
        }
    }
    salt[len] = '\0';
}

void hash_password(const char *password, const char *salt, char *hash_out) {
    // uses SHA-256
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char salted_password[256];

    // Combine password and salt
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt);

    SHA256((unsigned char *)salted_password, strlen(salted_password), hash);

    // Convert binary hash to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_out + (i * 2), "%02x", hash[i]);
    }
    hash_out[SHA256_DIGEST_LENGTH * 2] = '\0';
}

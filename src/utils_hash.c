#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // Isolated to this file
#include <external/openssl/rand.h>
#include <external/openssl/sha.h>

#include <stdio.h>

#include "utils_hash.h"

void generate_salt(char *salt, size_t len)
{
    if (RAND_bytes((unsigned char *)salt, len) != 1) {
        // Fallback to less secure method if RAND_bytes fails
        for (size_t i = 0; i < len; i++) {
            salt[i] = (char)(rand() % 256);
        }
    }
    salt[len] = '\0';
}

void hash_password(const char *password, const char *salt, char *hash_out)
{
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
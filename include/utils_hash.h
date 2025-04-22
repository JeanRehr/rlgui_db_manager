#pragma once
#include <stddef.h>

// Utility functions for hashing the user database
// Generates a salt with the given size
void generate_salt(char *salt, size_t len);

// Hashes the password with the given salt into the hash_out buffer
void hash_password(const char *password, const char *salt, char *hash_out);
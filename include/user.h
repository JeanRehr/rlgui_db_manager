#ifndef USER_H
#define USER_H

#include <time.h>

#include "CONSTANTS.h"

#define PASSWORD_HASH_LEN 64
#define SALT_LEN 32

struct user {
	int user_id;
	char username[MAX_INPUT];
	char password_hash[PASSWORD_HASH_LEN + 1]; // Store the hash, not the password
	char salt[SALT_LEN + 1]; // Salt for password hashing
	bool is_admin;
	bool reset_password;
	time_t created_at;
	time_t last_login;
};

#endif // USER_H
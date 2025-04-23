#ifndef USER_DB_H
#define USER_DB_H

#include <stdbool.h>

#include "db/db_manager.h"
#include "user.h"

int user_db_create_table(database *db);

// User management functions
int user_db_create(database *db, const char *username, const char *password, bool is_admin);

bool user_db_authenticate(database *db, const char *username, const char *password);

int user_db_delete(database *db, int user_id);

int user_db_update_password(database *db, int user_id, const char *new_password);

int user_db_update_admin_status(database *db, int user_id, bool is_admin);

// Query functions
bool user_db_user_exists(database *db, const char *username);

/* These two next fns will populate the struct with user info if available, exists early without populating if not */
int user_db_get_by_id(database *db, int user_id, struct user *user_out);

int user_db_get_by_username(database *db, const char *username, struct user *user_out);

int user_db_get_all(database *db);

#endif // USER_DB_H
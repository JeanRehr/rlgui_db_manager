#ifndef USER_DB_H
#define USER_DB_H

#include <stdbool.h>

#include "db/db_manager.h"
#include "user.h"

enum auth_result { AUTH_SUCCESS, AUTH_FAILURE, AUTH_NEED_PASSWORD_RESET };

int user_db_create_table(database *db);

int user_db_create_user(database *db, const char *username, bool is_admin, bool reset_password);

int user_db_create_admin(database *db);

enum auth_result user_db_authenticate(database *db, const char *username, const char *password);

int user_db_delete(database *db, const char *username);

int user_db_update_password(database *db, const char *username, const char *new_password);

int user_db_update_admin_status(database *db, const char *username, bool is_admin);

bool user_db_check_exists(database *db, const char *username);

int user_db_get_by_username(database *db, const char *username, struct user *user_out);

int user_db_change_username(database *db, const char *old_username, const char *new_username);

int user_db_get_all(database *db);

#endif // USER_DB_H
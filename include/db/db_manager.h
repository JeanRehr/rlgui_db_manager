#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <external/sqlite3.h>
#include <stdbool.h>

typedef struct database {
    sqlite3* db;
} database;

int db_init(database *db, const char *filename);

// Check if the database connection is valid
bool db_is_init(database *db);

void db_deinit(database *db);

#endif // DB_MANAGER_H
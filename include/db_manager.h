#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <sqlite3.h>

struct DatabaseManager {
    sqlite3* db;
};

int dbmanInit(struct DatabaseManager *dbManager, const char *name);

void dbmanClose(struct DatabaseManager *dbManager);

int dbmanInsertUser(struct DatabaseManager *dbManager, const char *name);

int dbmanRetrieveUser(struct DatabaseManager *dbManager);

#endif // DB_MANAGER_H
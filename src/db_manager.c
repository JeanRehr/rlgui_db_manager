#include <stdio.h>

#include "db_manager.h"

typedef struct DatabaseManager DatabaseManager;

int dbmanInit(DatabaseManager *dbManager, const char *name) {
    int rc = sqlite3_open(name, &dbManager->db);

    if (rc) {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(dbManager->db));
        return rc;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS Users(ID INTEGER PRIMARY KEY, Name TEXT);";
    char *errMsg = NULL;
    rc = sqlite3_exec(dbManager->db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}

void dbmanClose(DatabaseManager *dbManager) {
    if (dbManager->db) {
        sqlite3_close(dbManager->db);
    }
}

int dbmanInsertUser(DatabaseManager *dbManager, const char* name) {
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Users (Name) VALUES ('%s');", name);
    char *errMsg = NULL;

    int rc = sqlite3_exec(dbManager->db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }

    return SQLITE_OK;
}

int dbmanRetrieveUser(DatabaseManager *dbManager) {
    const char *sql = "SELECT ID, Name FROM Users;";
    sqlite3_stmt *stmt;

    // Prepare the SQL query
    int rc = sqlite3_prepare_v2(dbManager->db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(dbManager->db));
        return rc;
    }

    printf("User List:\n");
    printf("----------\n");

    // Execute the SQL statement and iterate over results
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);

        printf("ID: %d, Name: %s\n", id, name);
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to retrieve data: %s\n", sqlite3_errmsg(dbManager->db));
    }

    sqlite3_finalize(stmt);

    return SQLITE_OK;
}
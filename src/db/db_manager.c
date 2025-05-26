/**
 * @file db_manager.c
 * @brief Database manager implementation
 */
#include "db/db_manager.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "global/error_handling.h"

int db_init(database *db, const char *filename) {
    if (!db || !filename) {
        fprintf(stderr, "Database variable or string filename is NULL!\n");
        return SQLITE_ERROR;
    }

    int rc = sqlite3_open(filename, &db->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    return SQLITE_OK;
}

int db_init_with_tbl(database *db, const char *filename, int (*create_table)(database *)) {
    int rc = db_init(db, filename);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening database.\n");
        return rc;
    }

    if (!create_table) {
        fprintf(stderr, "Create table function not initialized");
        return SQLITE_ERROR;
    }

    if (create_table(db) != SQLITE_OK) {
        fprintf(stderr, "Error creating table in database %s.\n", filename);
        db_deinit(db);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

bool db_is_init(database *db) {
    if (!db || db->db == NULL) {
        return false;
    }

    return true;
}

void db_deinit(database *db) {
    if (db && db->db) {
        sqlite3_close(db->db);
        db->db = NULL; // setting pointer to null to prevent accidental reuse
    }
}

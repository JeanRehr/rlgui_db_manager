/**
 * @file db_manager.c
 * @brief Database manager implementation
 */
#include "db/db_manager.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error_handling.h"

int db_init(database *db, const char *filename) {
    int rc = sqlite3_open(filename, &db->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }

    return SQLITE_OK;
}

int db_init_with_tbl(database *db, const char *filename, int (*create_table)(database *)) {
    if (db_init(db, filename) != SQLITE_OK) {
        fprintf(stderr, "Error opening database %s.\n", filename);
        return ERROR_OPENING_DB;
    }

    if (create_table(db) != SQLITE_OK) {
        fprintf(stderr, "Error creating table in database %s.\n", filename);
        db_deinit(db);
        return ERROR_CREATING_TABLE_DB;
    }
    return SQLITE_OK;
}

bool db_is_init(database *db) {
    if (db->db == NULL) {
        return false;
    }

    return true;
}

/*
int db_get_first_table_row_count(database *db) {
    if (!db_is_init(db)) {
        fprintf(stderr, "Database connection is not initialized.\n");
        return -1;
    }

    // Statement for first table name
    const char *get_table_sql = "SELECT name FROM sqlite_master WHERE type='table' LIMIT 1;";
    sqlite3_stmt *stmt = NULL;
    char *table_name = NULL;
    int count = -1;

    // Get table name
    int rc = sqlite3_prepare_v2(db->db, get_table_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        goto cleanup;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        fprintf(stderr, "No tables found in database.\n");
        goto cleanup;
    }

    table_name = strdup((const char *)sqlite3_column_text(stmt, 0));
    if (!table_name) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }

    sqlite3_finalize(stmt);
    stmt = NULL;

    // Count rows in the table
    char *count_sql = sqlite3_mprintf("SELECT COUNT(*) FROM %q;", table_name);
    if (!count_sql) {
        fprintf(stderr, "Failed to allocate SQL query.\n");
        goto cleanup;
    }

    rc = sqlite3_prepare_v2(db->db, count_sql, -1, &stmt, NULL);
    sqlite3_free(count_sql);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare count statement: %s\n", sqlite3_errmsg(db->db));
        goto cleanup;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    } else {
        fprintf(stderr, "Failed to count rows: %s\n", sqlite3_errmsg(db->db));
    }

cleanup:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    if (table_name) {
        free(table_name);
    }

    return count;
}
*/

void db_deinit(database *db) {
    if (db->db) {
        sqlite3_close(db->db);
        db->db = NULL; // setting pointer to null to prevent accidental reuse
    }
}
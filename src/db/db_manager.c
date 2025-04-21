#include "db/db_manager.h"

#include <stddef.h>
#include <stdio.h>

#include "error_handling.h"

int db_init(database *db, const char *filename)
{
	int rc = sqlite3_open(filename, &db->db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

    return SQLITE_OK;
}

int db_init_with_tbl(database *db, const char *filename, int (*create_table)(database *))
{
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

bool db_is_init(database *db)
{
	if (db->db == NULL) {
		return false;
	}

    return true;
}

void db_deinit(database *db)
{
    if (db->db) {
        sqlite3_close(db->db);
        db->db = NULL; // setting pointer to null to prevent accidental reuse
    }
}
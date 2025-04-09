#include <stdio.h>
#include <stdbool.h>

#include "db_manager.h"
#include "Person.h"

#include <sqlite3.h>

const char *person_db_filename = "person_database.db";
const char *food_db_filename = "food_database.db";

int db_init()
{
	sqlite3 *db;
	char *errMsg = 0;
	int rc;

	// Initialize Person database
	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open person database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	const char *personTableSQL = "CREATE TABLE IF NOT EXISTS Person ("
								 "CPF TEXT PRIMARY KEY,"
								 "Name TEXT NOT NULL,"
								 "Age INTEGER NOT NULL,"
								 "HealthStatus TEXT,"
								 "Needs TEXT,"
								 "Gender INTEGER NOT NULL);";

	rc = sqlite3_exec(db, personTableSQL, 0, 0, &errMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error on init Person Table: %s\n", errMsg);
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return rc;
	}
	sqlite3_close(db);

	// Initialize Food database
	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open food database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	const char *foodTableSQL = "CREATE TABLE IF NOT EXISTS FoodItem ("
							   "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
							   "Name TEXT NOT NULL,"
							   "Quantity INTEGER NOT NULL,"
							   "ExpirationDate TEXT);";

	rc = sqlite3_exec(db, foodTableSQL, 0, 0, &errMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error on init Food Table: %s\n", errMsg);
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return rc;
	}
	sqlite3_close(db);
	return SQLITE_OK;
}

int db_insert_person(const char *cpf, const char *name, int age, const char *healthStatus, const char *needs,
					 int gender)
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	const char *sql = "INSERT INTO Person (CPF, Name, Age, HealthStatus, Needs, Gender) "
					  "VALUES (?, ?, ?, ?, ?, ?);";

	sqlite3_stmt *stmt;

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, age);
	sqlite3_bind_text(stmt, 4, healthStatus, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, needs, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, gender);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int db_update_person(const char *cpf, const char *name_input, int age_input, const char *healthStatus_input, const char *needs_input, int gender_input) {
    struct Person currentPerson;
    if (!db_get_person_by_cpf(cpf, &currentPerson)) {
        return SQLITE_ERROR;
    }

    // Decide which fields to use for update based on inputs
    const char *name = (name_input[0] != '\0') ? name_input : currentPerson.name;
    int age = (age_input > 0) ? age_input : currentPerson.age;
    const char *healthStatus = (healthStatus_input[0] != '\0') ? healthStatus_input : currentPerson.healthStatus;
    const char *needs = (needs_input[0] != '\0') ? needs_input : currentPerson.needs;
    int gender = (gender_input >= 0) ? gender_input : currentPerson.gender;

    sqlite3 *db;
    int rc;

    rc = sqlite3_open(person_db_filename, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    const char *sql = "UPDATE Person SET Name = ?, Age = ?, HealthStatus = ?, Needs = ?, Gender = ? WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, age);
    sqlite3_bind_text(stmt, 3, healthStatus, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, needs, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, gender);
    sqlite3_bind_text(stmt, 6, cpf, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}


bool db_check_cpf_exists(const char *cpf)
{
	sqlite3 *db;
	int rc;
	bool exists = false;

	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	}

	const char *sql = "SELECT 1 FROM Person WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}

	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		exists = true; // A row was found with the given CPF
	} else if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return exists;
}

bool db_get_person_by_cpf(const char *cpf, struct Person *person) {
    sqlite3 *db;
    int rc;

    rc = sqlite3_open(person_db_filename, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return false;
    }

    //const char *sql = "SELECT Name, Age, HealthStatus, Needs, Gender FROM Person WHERE CPF = ?;";
    const char *sql = "SELECT CPF, Name, Age, HealthStatus, Needs, Gender FROM Person WHERE CPF = ?;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strcpy(person->cpf, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(person->name, (const char *)sqlite3_column_text(stmt, 1));
        person->age = sqlite3_column_int(stmt, 2);
        strcpy(person->healthStatus, (const char *)sqlite3_column_text(stmt, 3));
        strcpy(person->needs, (const char *)sqlite3_column_text(stmt, 4));
        person->gender = sqlite3_column_int(stmt, 5);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    } else {
        fprintf(stderr, "No person found with CPF: %s\n", cpf);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

void db_get_all_persons()
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	const char *sql = "SELECT * FROM Person;";

	sqlite3_stmt *stmt;

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	printf(
		"+-----------------------------------------------------------------------------------------------------------"
		"--------------------------------------------------------+\n"
		"| CPF         | Name                                       | Age | HealthStatus                             "
		"  | Needs                                      | Gender |\n");
	printf(
		"+-------------+--------------------------------------------+-----+------------------------------------------"
		"--+--------------------------------------------+--------+\n");

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *cpf = (const char *)sqlite3_column_text(stmt, 0);
		const char *name = (const char *)sqlite3_column_text(stmt, 1);
		int age = sqlite3_column_int(stmt, 2);
		const char *healthStatus = (const char *)sqlite3_column_text(stmt, 3);
		const char *needs = (const char *)sqlite3_column_text(stmt, 4);
		int gender = sqlite3_column_int(stmt, 5);

		printf("| %-11s | %-42s | %-3d | %-42s | %-42s | %-6s |\n", cpf, name, age, healthStatus, needs,
			   (gender == 0 ? "Other" : (gender == 1 ? "Male" : "Female")));
		printf(
			"+-------------+--------------------------------------------+-----+--------------------------------------"
			"------+--------------------------------------------+--------+\n");
	}

	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
}

int db_insert_food(const char *name, int quantity, const char *expirationDate)
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	const char *sql = "INSERT INTO FoodItem (Name, Quantity, ExpirationDate) "
					  "VALUES (?, ?, ?);";

	sqlite3_stmt *stmt;

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, quantity);
	sqlite3_bind_text(stmt, 3, expirationDate, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc;
}
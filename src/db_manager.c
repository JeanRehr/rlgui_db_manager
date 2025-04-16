#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "db_manager.h"

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

	const char *foodTableSQL = "CREATE TABLE IF NOT EXISTS FoodBatch ("
							   "BatchId INTEGER PRIMARY KEY,"
							   "Name TEXT,"
							   "Quantity INTEGER NOT NULL,"
							   "IsPerishable INTEGER NOT NULL,"
							   "ExpirationDate TEXT,"
							   "DailyConsumptionRate REAL);";

	rc = sqlite3_exec(db, foodTableSQL, 0, 0, &errMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error on init FoodBatch Table: %s\n", errMsg);
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return rc;
	}
	sqlite3_close(db);
	return SQLITE_OK;
}

int db_insert_person(const char *cpf, const char *name, int age, const char *health_status, const char *needs,
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
	sqlite3_bind_text(stmt, 4, health_status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, needs, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, gender);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int db_update_person(const char *cpf, const char *name_input, int age_input, const char *health_status_input,
					 const char *needs_input, int gender_input)
{
	struct person currentPerson;
	if (!db_get_person_by_cpf(cpf, &currentPerson)) {
		return SQLITE_ERROR;
	}

	// Decide which fields to use for update based on inputs
	const char *name = (name_input[0] != '\0') ? name_input : currentPerson.name;
	int age = (age_input > 0) ? age_input : currentPerson.age;
	const char *health_status = (health_status_input[0] != '\0') ? health_status_input : currentPerson.health_status;
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
	sqlite3_bind_text(stmt, 3, health_status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, needs, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, gender);
	sqlite3_bind_text(stmt, 6, cpf, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_delete_person_by_cpf(const char *cpf)
{
	sqlite3 *db;
	int rc;

	// Open the database
	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	// Prepare the SQL delete statement
	const char *sql = "DELETE FROM Person WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	// Bind the CPF parameter
	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

	// Execute the DELETE statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db));
	}

	// Finalize the statement and close the database
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
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
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return exists;
}

bool db_get_person_by_cpf(const char *cpf, struct person *person)
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(person_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	}

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
		strcpy(person->health_status, (const char *)sqlite3_column_text(stmt, 3));
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
		const char *health_status = (const char *)sqlite3_column_text(stmt, 3);
		const char *needs = (const char *)sqlite3_column_text(stmt, 4);
		int gender = sqlite3_column_int(stmt, 5);

		printf("| %-11s | %-42s | %-3d | %-42s | %-42s | %-6s |\n", cpf, name, age, health_status, needs,
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

int db_insert_food_batch(int batch_id, const char *name, int quantity, bool isPerishable, const char *expirationDate,
						 float dailyConsumptionRate)
{
	sqlite3 *db;
	int rc;

	// Open the food database
	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	// SQL query to insert a new food batch
	const char *sql =
		"INSERT INTO FoodBatch (BatchId, Name, Quantity, IsPerishable, ExpirationDate, DailyConsumptionRate) "
		"VALUES (?, ?, ?, ?, ?, ?);";
	sqlite3_stmt *stmt;

	// Prepare the SQL statement
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	// Bind values to the prepared statement
	sqlite3_bind_int(stmt, 1, batch_id);
	sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, quantity);
	sqlite3_bind_int(stmt, 4, isPerishable ? 1 : 0);
	sqlite3_bind_text(stmt, 5, expirationDate, -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 6, dailyConsumptionRate);

	// Execute the SQL statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	// Finalize and clean up the prepared statement and close the database
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int db_update_food_batch(int batch_id, const char *name_input, int quantity_input, bool is_perishable_input,
						 const char *expiration_date_input, float daily_consumption_rate_input)
{
	struct foodbatch foodbatch;
	if (!db_get_food_by_batchid(batch_id, &foodbatch)) {
		return SQLITE_ERROR;
	}

	// Decide which fields to use for update based on inputs
	const char *name = (name_input[0] != '\0') ? name_input : foodbatch.name;
	int quantity = (quantity_input > 0) ? quantity_input : foodbatch.quantity;
	int is_perishable = (is_perishable_input > 0) ? is_perishable_input : foodbatch.is_perishable;
	const char *expiration_date =
		(expiration_date_input[0] != '\0') ? expiration_date_input : foodbatch.expiration_date;
	float daily_consumption_rate =
		(daily_consumption_rate_input >= 0) ? daily_consumption_rate_input : foodbatch.daily_consumption_rate;

	sqlite3 *db;
	int rc;

	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	const char *sql = "UPDATE FoodBatch SET Name = ?, Quantity = ?, IsPerishable = ?, ExpirationDate = ?, "
					  "DailyConsumptionRate = ? WHERE BatchId = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, quantity);
	sqlite3_bind_int(stmt, 3, is_perishable);
	sqlite3_bind_text(stmt, 4, expiration_date, -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 5, daily_consumption_rate);
	sqlite3_bind_int(stmt, 6, batch_id);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

bool db_get_food_by_batchid(int batch_id, struct foodbatch *foodbatch)
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	}

	const char *sql = "SELECT BatchId, Name, Quantity, IsPerishable, ExpirationDate, DailyConsumptionRate FROM "
					  "FoodBatch WHERE BatchId = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}

	sqlite3_bind_int(stmt, 1, batch_id);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		foodbatch->batch_id = sqlite3_column_int(stmt, 0);
		strcpy(foodbatch->name, (const char *)sqlite3_column_text(stmt, 1));
		foodbatch->quantity = sqlite3_column_int(stmt, 2);
		foodbatch->is_perishable = sqlite3_column_int(stmt, 3);
		strcpy(foodbatch->expiration_date, (const char *)sqlite3_column_text(stmt, 4));
		foodbatch->daily_consumption_rate = (float)sqlite3_column_double(stmt, 5);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return true;
	} else {
		fprintf(stderr, "No FoodBatch found with BatchId: %d\n", batch_id);
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return false;
}

bool db_check_batchid_exists(int batch_id)
{
	sqlite3 *db;
	int rc;
	bool exists = false;

	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	}

	const char *sql = "SELECT 1 FROM FoodBatch WHERE BatchId = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}

	sqlite3_bind_int(stmt, 1, batch_id);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		exists = true; // A row was found with the given CPF
	} else if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return exists;
}

void db_get_all_food()
{
	sqlite3 *db;
	int rc;

	rc = sqlite3_open(food_db_filename, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	const char *sql = "SELECT * FROM FoodBatch;";

	sqlite3_stmt *stmt;

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	printf("+---------------------------------------------------------------------------------------------------+\n");
	printf("| BatchId | Name                             | Quantity | Perishable | Expiration date | Daily Rate |\n");
	printf("+---------+----------------------------------+----------+------------+-----------------+------------+\n");

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int batch_id = sqlite3_column_int(stmt, 0);
		const char *name = (const char *)sqlite3_column_text(stmt, 1);
		int quantity = sqlite3_column_int(stmt, 2);
		int is_perishable = sqlite3_column_int(stmt, 3);
		const char *expiration_date = (const char *)sqlite3_column_text(stmt, 4);
		float daily_consumption_rate = (float)sqlite3_column_double(stmt, 5);

		printf("| %7d | %-32s | %-8d | %-10s | %-15s | %-10.2f |\n", batch_id, name, quantity,
			   (is_perishable == 0 ? "False" : "True"), expiration_date, daily_consumption_rate);
		printf(
			"+---------+----------------------------------+----------+------------+-----------------+------------+\n");
	}

	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
}
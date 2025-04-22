#include "db/resident_db.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int resident_db_create_table(database *db)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	const char *sql = "CREATE TABLE IF NOT EXISTS Resident ("
					  "CPF TEXT PRIMARY KEY,"
					  "Name TEXT NOT NULL,"
					  "Age INTEGER NOT NULL,"
					  "HealthStatus TEXT,"
					  "Needs TEXT,"
					  "MedicalAssistance INTEGER NOT NULL,"
					  "Gender INTEGER NOT NULL,"
					  "EntryDate TEXT);";

	char *errMsg = 0;
	int rc = sqlite3_exec(db->db, sql, 0, 0, &errMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error on init Resident table: %s\n", errMsg);
		sqlite3_free(errMsg);
		return rc;
	}
	return SQLITE_OK;
}

int resident_db_insert(database *db, const char *cpf, const char *name, int age, const char *health_status,
					   const char *needs, bool medical_assistance, int gender)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	const char *sql =
		"INSERT INTO Resident (CPF, Name, Age, HealthStatus, Needs, MedicalAssistance, Gender, EntryDate) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

	// Get current time for EntryDate
	time_t now;
	time(&now);

	struct tm *curr_time = localtime(&now);

	char date_string[11];
	snprintf(date_string, sizeof(date_string), "%04d-%02d-%02d", curr_time->tm_year + 1900, curr_time->tm_mon + 1,
			 curr_time->tm_mday);

	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, age);
	sqlite3_bind_text(stmt, 4, health_status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, needs, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, medical_assistance ? 1 : 0);
	sqlite3_bind_int(stmt, 7, gender);
	sqlite3_bind_text(stmt, 8, date_string, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db->db));
	}

	// Finalize statement
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

int resident_db_update(database *db, const char *cpf, const char *name_input, int age_input,
					   const char *health_status_input, const char *needs_input, int medical_assistance_input,
					   int gender_input)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	struct resident currentResident = {0};
	int rc = resident_db_get_by_cpf(db, cpf, &currentResident);
	
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Not possible to get resident by cpf: %d on function %s, line %d: %s\n", cpf, __func__,
				__LINE__, sqlite3_errmsg(db->db));
		return rc;
	}

	// Decide which fields to use for update based on inputs
	const char *name = (name_input[0] != '\0') ? name_input : currentResident.name;
	int age = (age_input > 0) ? age_input : currentResident.age;
	const char *health_status = (health_status_input[0] != '\0') ? health_status_input : currentResident.health_status;
	const char *needs = (needs_input[0] != '\0') ? needs_input : currentResident.needs;
	int medical_assistance =
		(medical_assistance_input > 0) ? medical_assistance_input : currentResident.medical_assistance;
	int gender = (gender_input >= 0) ? gender_input : currentResident.gender;

	const char *sql =
		"UPDATE Resident SET Name = ?, Age = ?, HealthStatus = ?, Needs = ?, MedicalAssistance = ?, Gender "
		"= ? WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, age);
	sqlite3_bind_text(stmt, 3, health_status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, needs, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, medical_assistance ? 1 : 0);
	sqlite3_bind_int(stmt, 6, gender);
	sqlite3_bind_text(stmt, 7, cpf, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db->db));
	}

	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int resident_db_delete_by_cpf(database *db, const char *cpf)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	// Prepare the SQL delete statement
	const char *sql = "DELETE FROM Resident WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

	// Bind the CPF parameter
	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

	// Execute the DELETE statement
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute delete statement: %s\n", sqlite3_errmsg(db->db));
	}

	// Finalize the statement
	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}

bool resident_db_check_cpf_exists(database *db, const char *cpf)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return false;
	}

	const char *sql = "SELECT 1 FROM Resident WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
		return false;
	}

	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

	bool exists = false;
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		exists = true; // A row was found with the given CPF
	} else if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute statement on function %s, line %d: %s\n", __func__, __LINE__,
				sqlite3_errmsg(db->db));
	}

	sqlite3_finalize(stmt);
	return exists;
}

int resident_db_get_by_cpf(database *db, const char *cpf, struct resident *resident)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	const char *sql =
		"SELECT CPF, Name, Age, HealthStatus, Needs, MedicalAssistance, Gender, EntryDate FROM Resident WHERE CPF = ?;";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, cpf, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		strcpy(resident->cpf, (const char *)sqlite3_column_text(stmt, 0));
		strcpy(resident->name, (const char *)sqlite3_column_text(stmt, 1));
		resident->age = sqlite3_column_int(stmt, 2);
		strcpy(resident->health_status, (const char *)sqlite3_column_text(stmt, 3));
		strcpy(resident->needs, (const char *)sqlite3_column_text(stmt, 4));
		resident->medical_assistance = sqlite3_column_int(stmt, 5);
		resident->gender = sqlite3_column_int(stmt, 6);
		strcpy(resident->entry_date, (const char *)sqlite3_column_text(stmt, 7));
		rc = SQLITE_OK; // Found and read successfully
	} else if (rc == SQLITE_DONE) {
		fprintf(stderr, "No resident found with CPF: %s\n", cpf);
		rc = SQLITE_NOTFOUND;
	} else {
		fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db->db));
	}

	sqlite3_finalize(stmt);
	return rc;
}

int resident_db_get_all(database *db)
{
	if (!db_is_init(db)) {
		fprintf(stderr, "Database connection is not initialized.\n");
		return SQLITE_ERROR;
	}

	const char *sql = "SELECT * FROM Resident;";

	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
		return rc;
	}

	printf(
		"+-----------------------------------------------------------------------------------------------------------"
		"-----------------------------------------------------------------------------------------------+\n"
		"| CPF         | Name                                       | Age | HealthStatus                             "
		"  | Needs                                      | Need Medical Assistance | Gender | Entry Date |\n");
	printf(
		"+-------------+--------------------------------------------+-----+------------------------------------------"
		"--+--------------------------------------------+-------------------------+--------+------------+\n");

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *cpf = (const char *)sqlite3_column_text(stmt, 0);
		const char *name = (const char *)sqlite3_column_text(stmt, 1);
		int age = sqlite3_column_int(stmt, 2);
		const char *health_status = (const char *)sqlite3_column_text(stmt, 3);
		const char *needs = (const char *)sqlite3_column_text(stmt, 4);
		int medical_assistance = sqlite3_column_int(stmt, 5);
		int gender = sqlite3_column_int(stmt, 6);
		const char *entry_date = (const char *)sqlite3_column_text(stmt, 7);

		printf("| %-11s | %-42s | %-3d | %-42s | %-42s | %-23s | %-6s | %-10s |\n", cpf, name, age, health_status,
			   needs, (medical_assistance == 0 ? "False" : "true"),
			   (gender == 0 ? "Other" : (gender == 1 ? "Male" : "Female")), entry_date);
		printf("+-------------+--------------------------------------------+-----+-------------------------------------"
			   "-----"
			   "--+--------------------------------------------+-------------------------+--------+------------+\n");
	}

	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db->db));
	}

	sqlite3_finalize(stmt);
	return rc == SQLITE_DONE ? SQLITE_OK : rc; // Return based on step result
}
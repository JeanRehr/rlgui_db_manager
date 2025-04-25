// Including raylib + raygui with #define RAYGUI_IMPLEMENTATION
// As makefile link all src/ files expect main.c for the tests directory
// It won't see the RAYGUI_IMPLEMENTATION and will complain
#include <external/raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raylib/raygui.h>

#include <external/sqlite3/sqlite3.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "db/db_manager.h"
#include "db/foodbatch_db.h"
#include "db/resident_db.h"
#include "db/user_db.h"

// TEST DB RESIDENT START

void test_resident_db_insert()
{
	database test_resident_db;
	db_init_with_tbl(&test_resident_db, "test_resident_db.db", resident_db_create_table);

	const char *test_cpf = "01234567890";
	char *test_name = "Test Name";
	int test_age = 10;
	char *test_health_status = "Test Health Status";
	char *test_needs = "Test Needs";
	bool test_medical_assistance = false;
	enum gender test_gender = GENDER_OTHER;

	printf("Attempting to insert a resident with the following values:\n"
		   "CPF: %s\n"
		   "Name: %s\n"
		   "Age: %d\n"
		   "Health Status: %s\n"
		   "Needs: %s\n"
		   "Medical Assistance: %s\n"
		   "Gender: %s\n",
		   test_cpf, test_name, test_age, test_health_status, test_needs,
		   test_medical_assistance == 0 ? "False" : "True",
		   test_gender == 0	  ? "Gender Other"
		   : test_gender == 1 ? "Gender Male"
							  : "Gender Female");

	int rc = resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
								test_medical_assistance, test_gender);

	assert(rc == SQLITE_OK);
	printf("Inserted resident successfully.\n");

	printf("Attempting to insert the same resident again with the same CPF.\n");

	rc = resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
							test_medical_assistance, test_gender);

	assert(rc != SQLITE_OK);
	printf("Attempt to insert the same resident was unsuccessful.\n");

	db_deinit(&test_resident_db);

	if (remove("test_resident_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Resident database insertion test passed successfully.\n");
}

void test_resident_db_retrieve()
{
	database test_resident_db;
	db_init_with_tbl(&test_resident_db, "test_resident_db.db", resident_db_create_table);

	const char *test_cpf = "01234567890";
	char *test_name = "Test Name";
	int test_age = 10;
	char *test_health_status = "Test Health Status";
	char *test_needs = "Test Needs";
	bool test_medical_assistance = false;
	enum gender test_gender = GENDER_OTHER;

	printf("Inserting a resident with the following values:\n"
		   "CPF: %s\n"
		   "Name: %s\n"
		   "Age: %d\n"
		   "Health Status: %s\n"
		   "Needs: %s\n"
		   "Medical Assistance: %s\n"
		   "Gender: %s\n",
		   test_cpf, test_name, test_age, test_health_status, test_needs,
		   test_medical_assistance == 0 ? "False" : "True",
		   test_gender == 0	  ? "Gender Other"
		   : test_gender == 1 ? "Gender Male"
							  : "Gender Female");

	int rc = resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
								test_medical_assistance, test_gender);

	struct resident test_resident = {0};

	printf("Attempting to retrieve the above resident and insert it into a resident structure\n");

	rc = resident_db_get_by_cpf(&test_resident_db, test_cpf, &test_resident);

	assert(rc == SQLITE_OK);
	assert(strcmp(test_resident.cpf, test_cpf) == 0);
	assert(strcmp(test_resident.name, test_name) == 0);
	assert(test_resident.age == test_age);
	assert(strcmp(test_resident.health_status, test_health_status) == 0);
	assert(strcmp(test_resident.needs, test_needs) == 0);
	assert(test_resident.medical_assistance == test_medical_assistance);
	assert(test_resident.gender == test_gender);

	printf("Retrieve successful\n");

	db_deinit(&test_resident_db);

	if (remove("test_resident_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Resident database retrieval test passed.\n");
}

void test_resident_db_update()
{
	database test_resident_db;
	db_init_with_tbl(&test_resident_db, "test_resident_db.db", resident_db_create_table);

	const char *test_cpf = "01234567890";
	char *test_name = "Test Name";
	int test_age = 10;
	char *test_health_status = "Test Health Status";
	char *test_needs = "Test Needs";
	bool test_medical_assistance = false;
	enum gender test_gender = GENDER_OTHER;

	printf("Attempting to update a non-existent resident.\n");
	int rc = resident_db_update(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
								test_medical_assistance, test_gender);

	assert(rc != SQLITE_OK);
	printf("Updating a non-existent user was unsuccessful.\n");

	printf("Inserting a resident with the following values:\n"
		   "CPF: %s\n"
		   "Name: %s\n"
		   "Age: %d\n"
		   "Health Status: %s\n"
		   "Needs: %s\n"
		   "Medical Assistance: %s\n"
		   "Gender: %s\n",
		   test_cpf, test_name, test_age, test_health_status, test_needs,
		   test_medical_assistance == 0 ? "False" : "True",
		   test_gender == 0	  ? "Gender Other"
		   : test_gender == 1 ? "Gender Male"
							  : "Gender Female");

	rc = resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
							test_medical_assistance, test_gender);

	const char *updated_name = "UPDATED NAME";
	printf("Updating the name of the resident to %s\n", updated_name);
	rc = resident_db_update(&test_resident_db, test_cpf, updated_name, test_age, test_health_status, test_needs,
							test_medical_assistance, test_gender);

	assert(rc == SQLITE_OK);

	printf("Update operation was successful.\n");

	printf("Retrieving the resident to test the name change.\n");

	struct resident test_resident = {0};
	resident_db_get_by_cpf(&test_resident_db, test_cpf, &test_resident);

	assert(strcmp(test_resident.name, updated_name) == 0);
	printf("Retrieved resident name has the updated name.\n");

	db_deinit(&test_resident_db);

	if (remove("test_resident_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Resident database update test passed successfully.\n");
}

void test_resident_db_check_cpf_exists()
{
	database test_resident_db;
	db_init_with_tbl(&test_resident_db, "test_resident_db.db", resident_db_create_table);

	const char *test_cpf = "01234567890";
	char *test_name = "Test Name";
	int test_age = 10;
	char *test_health_status = "Test Health Status";
	char *test_needs = "Test Needs";
	bool test_medical_assistance = false;
	enum gender test_gender = GENDER_OTHER;

	printf("Attempting to check if a non-existent resident exists.\n");

	bool exists = resident_db_check_cpf_exists(&test_resident_db, test_cpf);

	assert(exists == false);

	printf("Doesn't exist.\n");

	printf("Inserting a resident with the following values:\n"
		   "CPF: %s\n"
		   "Name: %s\n"
		   "Age: %d\n"
		   "Health Status: %s\n"
		   "Needs: %s\n"
		   "Medical Assistance: %s\n"
		   "Gender: %s\n",
		   test_cpf, test_name, test_age, test_health_status, test_needs,
		   test_medical_assistance == 0 ? "False" : "True",
		   test_gender == 0	  ? "Gender Other"
		   : test_gender == 1 ? "Gender Male"
							  : "Gender Female");

	resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
					   test_medical_assistance, test_gender);

	printf("Attempting to check if the inserted resident exists.\n");

	exists = resident_db_check_cpf_exists(&test_resident_db, test_cpf);

	assert(exists == true);

	printf("Exist.\n");

	db_deinit(&test_resident_db);

	if (remove("test_resident_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Resident database check cpf exists test passed successfully.\n");
}

void test_resident_db_delete_by_cpf()
{
	database test_resident_db;
	db_init_with_tbl(&test_resident_db, "test_resident_db.db", resident_db_create_table);

	const char *test_cpf = "01234567890";
	char *test_name = "Test Name";
	int test_age = 10;
	char *test_health_status = "Test Health Status";
	char *test_needs = "Test Needs";
	bool test_medical_assistance = false;
	enum gender test_gender = GENDER_OTHER;

	printf("Inserting a resident with the following values:\n"
		   "CPF: %s\n"
		   "Name: %s\n"
		   "Age: %d\n"
		   "Health Status: %s\n"
		   "Needs: %s\n"
		   "Medical Assistance: %s\n"
		   "Gender: %s\n",
		   test_cpf, test_name, test_age, test_health_status, test_needs,
		   test_medical_assistance == 0 ? "False" : "True",
		   test_gender == 0	  ? "Gender Other"
		   : test_gender == 1 ? "Gender Male"
							  : "Gender Female");

	resident_db_insert(&test_resident_db, test_cpf, test_name, test_age, test_health_status, test_needs,
					   test_medical_assistance, test_gender);

	printf("Attempting to delete the inserted resident.\n");

	int rc = resident_db_delete_by_cpf(&test_resident_db, test_cpf);

	assert(rc == SQLITE_OK);
	printf("Operation successful.\n");

	printf("Check if the removed user still exists.\n");

	bool exists = resident_db_check_cpf_exists(&test_resident_db, test_cpf);

	assert(exists == false);

	printf("Operation successful.\n");

	db_deinit(&test_resident_db);

	if (remove("test_resident_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Resident database delete test passed successfully.\n");
}

// TEST DB RESIDENT END

// TEST DB FOODBATCH START

void test_foodbatch_db_insert()
{
	database test_foodbatch_db;
	db_init_with_tbl(&test_foodbatch_db, "test_foodbatch_db.db", foodbatch_db_create_table);

	int test_batch_id = 1;
	char *test_name = "Test Food";
	int test_quantity = 100;
	bool test_is_perishable = true;
	char *test_expiration_date = "2023-12-31";
	float test_daily_consumption_rate = 5.0;

	printf("Attempting to insert a food batch with the following values:\n"
		   "Batch ID: %d\n"
		   "Name: %s\n"
		   "Quantity: %d\n"
		   "Is Perishable: %s\n"
		   "Expiration Date: %s\n"
		   "Daily Consumption Rate: %.2f\n",
		   test_batch_id, test_name, test_quantity, test_is_perishable ? "True" : "False", test_expiration_date,
		   test_daily_consumption_rate);

	int rc = foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
								 test_expiration_date, test_daily_consumption_rate);

	assert(rc == SQLITE_OK);
	printf("Inserted food batch successfully.\n");

	printf("Attempting to insert the same food batch again with the same Batch ID.\n");

	rc = foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
							 test_expiration_date, test_daily_consumption_rate);

	assert(rc != SQLITE_OK);
	printf("Attempt to insert the same food batch was unsuccessful.\n");

	db_deinit(&test_foodbatch_db);

	if (remove("test_foodbatch_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Food batch database insertion test passed successfully.\n");
}

void test_foodbatch_db_retrieve()
{
	database test_foodbatch_db;
	db_init_with_tbl(&test_foodbatch_db, "test_foodbatch_db.db", foodbatch_db_create_table);

	int test_batch_id = 1;
	char *test_name = "Test Food";
	int test_quantity = 100;
	bool test_is_perishable = true;
	char *test_expiration_date = "2023-12-31";
	float test_daily_consumption_rate = 5.0;

	printf("Inserting a food batch with the following values:\n"
		   "Batch ID: %d\n"
		   "Name: %s\n"
		   "Quantity: %d\n"
		   "Is Perishable: %s\n"
		   "Expiration Date: %s\n"
		   "Daily Consumption Rate: %.2f\n",
		   test_batch_id, test_name, test_quantity, test_is_perishable ? "True" : "False", test_expiration_date,
		   test_daily_consumption_rate);

	int rc = foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
								 test_expiration_date, test_daily_consumption_rate);

	struct foodbatch test_foodbatch = {0};

	printf("Attempting to retrieve the above food batch and insert it into a foodbatch structure\n");

	rc = foodbatch_db_get_by_batchid(&test_foodbatch_db, test_batch_id, &test_foodbatch);

	assert(rc == SQLITE_OK);
	assert(test_foodbatch.batch_id == test_batch_id);
	assert(strcmp(test_foodbatch.name, test_name) == 0);
	assert(test_foodbatch.quantity == test_quantity);
	assert(test_foodbatch.is_perishable == test_is_perishable);
	assert(strcmp(test_foodbatch.expiration_date, test_expiration_date) == 0);
	assert(test_foodbatch.daily_consumption_rate == test_daily_consumption_rate);

	printf("Retrieve successful\n");

	db_deinit(&test_foodbatch_db);

	if (remove("test_foodbatch_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Food batch database retrieval test passed.\n");
}

void test_foodbatch_db_update()
{
	database test_foodbatch_db;
	db_init_with_tbl(&test_foodbatch_db, "test_foodbatch_db.db", foodbatch_db_create_table);

	int test_batch_id = 1;
	char *test_name = "Test Food";
	int test_quantity = 100;
	bool test_is_perishable = true;
	char *test_expiration_date = "2023-12-31";
	float test_daily_consumption_rate = 5.0;

	printf("Attempting to update a non-existent food batch.\n");
	int rc = foodbatch_db_update(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
								 test_expiration_date, test_daily_consumption_rate);

	assert(rc != SQLITE_OK);
	printf("Updating a non-existent food batch was unsuccessful.\n");

	printf("Inserting a food batch with the following values:\n"
		   "Batch ID: %d\n"
		   "Name: %s\n"
		   "Quantity: %d\n"
		   "Is Perishable: %s\n"
		   "Expiration Date: %s\n"
		   "Daily Consumption Rate: %.2f\n",
		   test_batch_id, test_name, test_quantity, test_is_perishable ? "True" : "False", test_expiration_date,
		   test_daily_consumption_rate);

	rc = foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
							 test_expiration_date, test_daily_consumption_rate);

	char *updated_name = "UPDATED FOOD";
	int updated_quantity = 200;
	printf("Updating the name of the food batch to %s and quantity to %d\n", updated_name, updated_quantity);
	rc = foodbatch_db_update(&test_foodbatch_db, test_batch_id, updated_name, updated_quantity, test_is_perishable,
							 test_expiration_date, test_daily_consumption_rate);

	assert(rc == SQLITE_OK);
	printf("Update operation was successful.\n");

	printf("Retrieving the food batch to test the changes.\n");

	struct foodbatch test_foodbatch = {0};
	foodbatch_db_get_by_batchid(&test_foodbatch_db, test_batch_id, &test_foodbatch);

	assert(strcmp(test_foodbatch.name, updated_name) == 0);
	assert(test_foodbatch.quantity == updated_quantity);
	printf("Retrieved food batch has the updated values.\n");

	db_deinit(&test_foodbatch_db);

	if (remove("test_foodbatch_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Food batch database update test passed successfully.\n");
}

void test_foodbatch_db_check_batchid_exists()
{
	database test_foodbatch_db;
	db_init_with_tbl(&test_foodbatch_db, "test_foodbatch_db.db", foodbatch_db_create_table);

	int test_batch_id = 1;
	char *test_name = "Test Food";
	int test_quantity = 100;
	bool test_is_perishable = true;
	char *test_expiration_date = "2023-12-31";
	float test_daily_consumption_rate = 5.0;

	printf("Attempting to check if a non-existent food batch exists.\n");

	bool exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, test_batch_id);

	assert(exists == false);
	printf("Doesn't exist.\n");

	printf("Inserting a food batch with the following values:\n"
		   "Batch ID: %d\n"
		   "Name: %s\n"
		   "Quantity: %d\n"
		   "Is Perishable: %s\n"
		   "Expiration Date: %s\n"
		   "Daily Consumption Rate: %.2f\n",
		   test_batch_id, test_name, test_quantity, test_is_perishable ? "True" : "False", test_expiration_date,
		   test_daily_consumption_rate);

	foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
						test_expiration_date, test_daily_consumption_rate);

	printf("Attempting to check if the inserted food batch exists.\n");

	exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, test_batch_id);

	assert(exists == true);
	printf("Exists.\n");

	db_deinit(&test_foodbatch_db);

	if (remove("test_foodbatch_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Food batch database check batch ID exists test passed successfully.\n");
}

void test_foodbatch_db_delete_by_id()
{
	database test_foodbatch_db;
	db_init_with_tbl(&test_foodbatch_db, "test_foodbatch_db.db", foodbatch_db_create_table);

	int test_batch_id = 1;
	char *test_name = "Test Food";
	int test_quantity = 100;
	bool test_is_perishable = true;
	char *test_expiration_date = "2023-12-31";
	float test_daily_consumption_rate = 5.0;

	printf("Inserting a food batch with the following values:\n"
		   "Batch ID: %d\n"
		   "Name: %s\n"
		   "Quantity: %d\n"
		   "Is Perishable: %s\n"
		   "Expiration Date: %s\n"
		   "Daily Consumption Rate: %.2f\n",
		   test_batch_id, test_name, test_quantity, test_is_perishable ? "True" : "False", test_expiration_date,
		   test_daily_consumption_rate);

	foodbatch_db_insert(&test_foodbatch_db, test_batch_id, test_name, test_quantity, test_is_perishable,
						test_expiration_date, test_daily_consumption_rate);

	printf("Attempting to delete the inserted food batch.\n");

	int rc = foodbatch_db_delete_by_id(&test_foodbatch_db, test_batch_id);

	assert(rc == SQLITE_OK);
	printf("Operation successful.\n");

	printf("Check if the removed food batch still exists.\n");

	bool exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, test_batch_id);

	assert(exists == false);
	printf("Operation successful.\n");

	db_deinit(&test_foodbatch_db);

	if (remove("test_foodbatch_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("Food batch database delete test passed successfully.\n");
}

// TEST DB FOODBATCH END

// TEST DB USER START

void test_user_db_create()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	bool test_is_admin = false;

	printf("Attempting to create a user with the following values:\n"
		   "Username: %s\n"
		   "Is Admin: %s\n",
		   test_username, test_is_admin ? "True" : "False");

	int rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);

	assert(rc == SQLITE_OK);
	printf("Created user successfully.\n");

	printf("Attempting to create the same user again with the same username.\n");

	rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);

	assert(rc != SQLITE_OK);
	printf("Attempt to create the same user was unsuccessful.\n");

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User database creation test passed successfully.\n");
}

void test_user_db_authenticate()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	const char *wrong_password = "wrongpassword";
	bool test_is_admin = false;

	printf("Creating a test user for authentication\n");
	user_db_create(&test_user_db, test_username, test_password, test_is_admin);

	printf("Attempting to authenticate with correct credentials\n");
	bool auth_result = user_db_authenticate(&test_user_db, test_username, test_password);
	assert(auth_result == true);
	printf("Authentication successful with correct credentials\n");

	printf("Attempting to authenticate with wrong password\n");
	auth_result = user_db_authenticate(&test_user_db, test_username, wrong_password);
	assert(auth_result == false);
	printf("Authentication failed with wrong password as expected\n");

	printf("Attempting to authenticate with non-existent user\n");
	auth_result = user_db_authenticate(&test_user_db, "nonexistent", test_password);
	assert(auth_result == false);
	printf("Authentication failed with non-existent user as expected\n");

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User authentication test passed successfully.\n");
}

void test_user_db_get_user()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	bool test_is_admin = false;

	printf("Creating a test user for retrieval tests\n");
	int rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);
	assert(rc == SQLITE_OK);

	// First get the user ID by username
	struct user user_by_username = {0};
	rc = user_db_get_by_username(&test_user_db, test_username, &user_by_username);
	assert(rc == SQLITE_OK);
	printf("Retrieved user by username successfully\n");

	// Now get the same user by ID
	struct user user_by_id = {0};
	rc = user_db_get_by_id(&test_user_db, user_by_username.user_id, &user_by_id);
	assert(rc == SQLITE_OK);
	printf("Retrieved user by ID successfully\n");

	// Verify both retrievals got the same user
	assert(user_by_username.user_id == user_by_id.user_id);
	assert(strcmp(user_by_username.username, user_by_id.username) == 0);
	assert(user_by_username.is_admin == user_by_id.is_admin);
	printf("Both retrieval methods returned the same user data\n");

	// Test non-existent user
	struct user non_existent = {0};
	rc = user_db_get_by_username(&test_user_db, "nonexistent", &non_existent);
	assert(rc != SQLITE_OK);
	printf("Failed to retrieve non-existent user as expected\n");

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User retrieval tests passed successfully.\n");
}

void test_user_db_update_functions()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	const char *new_password = "newpassword";
	bool test_is_admin = false;
	bool new_admin_status = true;

	printf("Creating a test user for update tests\n");
	int rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);
	assert(rc == SQLITE_OK);

	// Get the user ID
	struct user test_user = {0};
	rc = user_db_get_by_username(&test_user_db, test_username, &test_user);
	assert(rc == SQLITE_OK);

	printf("Testing password update\n");
	rc = user_db_update_password(&test_user_db, test_user.user_id, new_password);
	assert(rc == SQLITE_OK);

	// Verify password was changed
	bool auth_result = user_db_authenticate(&test_user_db, test_username, new_password);
	assert(auth_result == true);
	printf("Password update successful\n");

	printf("Testing admin status update\n");
	rc = user_db_update_admin_status(&test_user_db, test_user.user_id, new_admin_status);
	assert(rc == SQLITE_OK);

	// Verify admin status was changed
	rc = user_db_get_by_id(&test_user_db, test_user.user_id, &test_user);
	assert(rc == SQLITE_OK);
	assert(test_user.is_admin == new_admin_status);
	printf("Admin status update successful\n");

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User update tests passed successfully.\n");
}

void test_user_db_delete()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	bool test_is_admin = false;

	printf("Creating a test user for deletion test\n");
	int rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);
	assert(rc == SQLITE_OK);

	// Get the user ID
	struct user test_user = {0};
	rc = user_db_get_by_username(&test_user_db, test_username, &test_user);
	assert(rc == SQLITE_OK);

	printf("Checking user exists before deletion\n");
	bool exists = user_db_user_exists(&test_user_db, test_username);
	assert(exists == true);

	printf("Attempting to delete the user\n");
	rc = user_db_delete(&test_user_db, test_user.user_id);
	assert(rc == SQLITE_OK);

	printf("Checking user no longer exists after deletion\n");
	exists = user_db_user_exists(&test_user_db, test_username);
	assert(exists == false);

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User deletion tests passed successfully.\n");
}

void test_user_db_user_exists()
{
	database test_user_db;
	db_init_with_tbl(&test_user_db, "test_user_db.db", user_db_create_table);

	const char *test_username = "testuser";
	const char *test_password = "testpassword";
	bool test_is_admin = false;

	printf("Checking non-existent user\n");
	bool exists = user_db_user_exists(&test_user_db, "nonexistent");
	assert(exists == false);

	printf("Creating a test user\n");
	int rc = user_db_create(&test_user_db, test_username, test_password, test_is_admin);
	assert(rc == SQLITE_OK);

	printf("Checking existing user\n");
	exists = user_db_user_exists(&test_user_db, test_username);
	assert(exists == true);

	db_deinit(&test_user_db);

	if (remove("test_user_db.db") == 0) {
		printf("Test database file deleted successfully.\n");
	}

	printf("User existence check tests passed successfully.\n");
}

// TEST DB USER END

int main()
{
	test_resident_db_insert();
	test_resident_db_retrieve();
	test_resident_db_update();
	test_resident_db_check_cpf_exists();
	test_resident_db_delete_by_cpf();

	test_foodbatch_db_insert();
	test_foodbatch_db_retrieve();
	test_foodbatch_db_update();
	test_foodbatch_db_check_batchid_exists();
	test_foodbatch_db_delete_by_id();

	test_user_db_create();
	test_user_db_authenticate();
	test_user_db_get_user();
	test_user_db_update_functions();
	test_user_db_delete();
	test_user_db_user_exists();

	return 0;
}
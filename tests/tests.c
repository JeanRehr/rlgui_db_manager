// Including raylib + raygui with #define RAYGUI_IMPLEMENTATION
// As makefile link all src/ files expect main.c for the tests directory
// It won't see the RAYGUI_IMPLEMENTATION and will complain
#include <external/raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raylib/raygui.h>

#include <external/sqlite3/sqlite3.h>

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#include "db/db_manager.h"
#include "db/foodbatch_db.h"
#include "db/resident_db.h"
#include "db/user_db.h"

// Global context structure
struct test_cleanup_ctx {
    const char *db_filename;
    database *db_handle;
};

static struct test_cleanup_ctx cleanup_ctx = { 0 };

// Signal handler that cleans up
void cleanup_handler(int sig) {
    if (cleanup_ctx.db_handle) {
        db_deinit(cleanup_ctx.db_handle);
    }

    if (cleanup_ctx.db_filename) {
        remove(cleanup_ctx.db_filename);
    }

    fprintf(stderr, "Test failed with signal %d. Cleaned up resources.\n", sig);
    exit(EXIT_FAILURE);
}

// Setup function
void setup_cleanup(const char *filename, database *db) {
    // Set up signal handlers
    signal(SIGABRT, cleanup_handler); // assertion failure signal

    // Store cleanup context
    cleanup_ctx.db_filename = filename;
    cleanup_ctx.db_handle = db;
}

// Teardown function
void teardown_cleanup() {
    // Normal cleanup
    if (cleanup_ctx.db_handle) {
        db_deinit(cleanup_ctx.db_handle);
    }

    if (cleanup_ctx.db_filename) {
        remove(cleanup_ctx.db_filename);
    }

    // Reset context
    cleanup_ctx = (struct test_cleanup_ctx) { 0 };

    // Reset signal handler to default
    signal(SIGABRT, SIG_DFL);
}

// TEST DB RESIDENT START

void test_resident_db_insert() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    const char *test_cpf = "01234567890";
    char *test_name = "Test Name";
    int test_age = 10;
    char *test_health_status = "Test Health Status";
    char *test_needs = "Test Needs";
    bool test_medical_assistance = false;
    enum gender test_gender = GENDER_OTHER;

    printf(
        "Attempting to insert a resident with the following values:\n"
        "CPF: %s\n"
        "Name: %s\n"
        "Age: %d\n"
        "Health Status: %s\n"
        "Needs: %s\n"
        "Medical Assistance: %s\n"
        "Gender: %s\n",
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance == 0 ? "False" : "True",
        test_gender == 0     ? "Gender Other" :
            test_gender == 1 ? "Gender Male" :
                               "Gender Female"
    );

    int rc = resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    assert(rc == SQLITE_OK);
    printf("Inserted resident successfully.\n");

    printf("Attempting to insert the same resident again with the same CPF.\n");

    rc = resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    assert(rc != SQLITE_OK);
    printf("Attempt to insert the same resident was unsuccessful.\n");

    teardown_cleanup();

    printf("Resident database insertion test passed successfully.\n");
}

void test_resident_db_retrieve() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    const char *test_cpf = "01234567890";
    char *test_name = "Test Name";
    int test_age = 10;
    char *test_health_status = "Test Health Status";
    char *test_needs = "Test Needs";
    bool test_medical_assistance = false;
    enum gender test_gender = GENDER_OTHER;

    printf(
        "Inserting a resident with the following values:\n"
        "CPF: %s\n"
        "Name: %s\n"
        "Age: %d\n"
        "Health Status: %s\n"
        "Needs: %s\n"
        "Medical Assistance: %s\n"
        "Gender: %s\n",
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance == 0 ? "False" : "True",
        test_gender == 0     ? "Gender Other" :
            test_gender == 1 ? "Gender Male" :
                               "Gender Female"
    );

    int rc = resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    struct resident test_resident = { 0 };

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

    teardown_cleanup();

    printf("Resident database retrieval test passed.\n");
}

void test_resident_db_update() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    const char *test_cpf = "01234567890";
    char *test_name = "Test Name";
    int test_age = 10;
    char *test_health_status = "Test Health Status";
    char *test_needs = "Test Needs";
    bool test_medical_assistance = false;
    enum gender test_gender = GENDER_OTHER;

    printf("Attempting to update a non-existent resident.\n");
    int rc = resident_db_update(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    assert(rc != SQLITE_OK);
    printf("Updating a non-existent user was unsuccessful.\n");

    printf(
        "Inserting a resident with the following values:\n"
        "CPF: %s\n"
        "Name: %s\n"
        "Age: %d\n"
        "Health Status: %s\n"
        "Needs: %s\n"
        "Medical Assistance: %s\n"
        "Gender: %s\n",
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance == 0 ? "False" : "True",
        test_gender == 0     ? "Gender Other" :
            test_gender == 1 ? "Gender Male" :
                               "Gender Female"
    );

    rc = resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    const char *updated_name = "UPDATED NAME";
    printf("Updating the name of the resident to %s\n", updated_name);
    rc = resident_db_update(
        &test_resident_db,
        test_cpf,
        updated_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    assert(rc == SQLITE_OK);

    printf("Update operation was successful.\n");

    printf("Retrieving the resident to test the name change.\n");

    struct resident test_resident = { 0 };
    resident_db_get_by_cpf(&test_resident_db, test_cpf, &test_resident);

    assert(strcmp(test_resident.name, updated_name) == 0);
    printf("Retrieved resident name has the updated name.\n");

    teardown_cleanup();

    printf("Resident database update test passed successfully.\n");
}

void test_resident_db_check_cpf_exists() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

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

    printf(
        "Inserting a resident with the following values:\n"
        "CPF: %s\n"
        "Name: %s\n"
        "Age: %d\n"
        "Health Status: %s\n"
        "Needs: %s\n"
        "Medical Assistance: %s\n"
        "Gender: %s\n",
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance == 0 ? "False" : "True",
        test_gender == 0     ? "Gender Other" :
            test_gender == 1 ? "Gender Male" :
                               "Gender Female"
    );

    resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    printf("Attempting to check if the inserted resident exists.\n");

    exists = resident_db_check_cpf_exists(&test_resident_db, test_cpf);

    assert(exists == true);

    printf("Exist.\n");

    teardown_cleanup();

    printf("Resident database check cpf exists test passed successfully.\n");
}

void test_resident_db_delete_by_cpf() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    const char *test_cpf = "01234567890";
    char *test_name = "Test Name";
    int test_age = 10;
    char *test_health_status = "Test Health Status";
    char *test_needs = "Test Needs";
    bool test_medical_assistance = false;
    enum gender test_gender = GENDER_OTHER;

    printf(
        "Inserting a resident with the following values:\n"
        "CPF: %s\n"
        "Name: %s\n"
        "Age: %d\n"
        "Health Status: %s\n"
        "Needs: %s\n"
        "Medical Assistance: %s\n"
        "Gender: %s\n",
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance == 0 ? "False" : "True",
        test_gender == 0     ? "Gender Other" :
            test_gender == 1 ? "Gender Male" :
                               "Gender Female"
    );

    resident_db_insert(
        &test_resident_db,
        test_cpf,
        test_name,
        test_age,
        test_health_status,
        test_needs,
        test_medical_assistance,
        test_gender
    );

    printf("Attempting to delete the inserted resident.\n");

    int rc = resident_db_delete_by_cpf(&test_resident_db, test_cpf);

    assert(rc == SQLITE_OK);
    printf("Operation successful.\n");

    printf("Check if the removed user still exists.\n");

    bool exists = resident_db_check_cpf_exists(&test_resident_db, test_cpf);

    assert(exists == false);

    printf("Operation successful.\n");

    teardown_cleanup();

    printf("Resident database delete test passed successfully.\n");
}

// TEST DB RESIDENT END

// TEST DB FOODBATCH START

void test_foodbatch_db_insert() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    int test_batch_id = 1;
    char *test_name = "Test Food";
    int test_quantity = 100;
    bool test_is_perishable = true;
    char *test_expiration_date = "2023-12-31";
    float test_daily_consumption_rate = 5.0;

    printf(
        "Attempting to insert a food batch with the following values:\n"
        "Batch ID: %d\n"
        "Name: %s\n"
        "Quantity: %d\n"
        "Is Perishable: %s\n"
        "Expiration Date: %s\n"
        "Daily Consumption Rate: %.2f\n",
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable ? "True" : "False",
        test_expiration_date,
        test_daily_consumption_rate
    );

    int rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    assert(rc == SQLITE_OK);
    printf("Inserted food batch successfully.\n");

    printf("Attempting to insert the same food batch again with the same Batch ID.\n");

    rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    assert(rc != SQLITE_OK);
    printf("Attempt to insert the same food batch was unsuccessful.\n");

    teardown_cleanup();

    printf("Food batch database insertion test passed successfully.\n");
}

void test_foodbatch_db_retrieve() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    int test_batch_id = 1;
    char *test_name = "Test Food";
    int test_quantity = 100;
    bool test_is_perishable = true;
    char *test_expiration_date = "2023-12-31";
    float test_daily_consumption_rate = 5.0;

    printf(
        "Inserting a food batch with the following values:\n"
        "Batch ID: %d\n"
        "Name: %s\n"
        "Quantity: %d\n"
        "Is Perishable: %s\n"
        "Expiration Date: %s\n"
        "Daily Consumption Rate: %.2f\n",
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable ? "True" : "False",
        test_expiration_date,
        test_daily_consumption_rate
    );

    int rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    struct foodbatch test_foodbatch = { 0 };

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

    teardown_cleanup();

    printf("Food batch database retrieval test passed.\n");
}

void test_foodbatch_db_update() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    int test_batch_id = 1;
    char *test_name = "Test Food";
    int test_quantity = 100;
    bool test_is_perishable = true;
    char *test_expiration_date = "2023-12-31";
    float test_daily_consumption_rate = 5.0;

    printf("Attempting to update a non-existent food batch.\n");
    int rc = foodbatch_db_update(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    assert(rc != SQLITE_OK);
    printf("Updating a non-existent food batch was unsuccessful.\n");

    printf(
        "Inserting a food batch with the following values:\n"
        "Batch ID: %d\n"
        "Name: %s\n"
        "Quantity: %d\n"
        "Is Perishable: %s\n"
        "Expiration Date: %s\n"
        "Daily Consumption Rate: %.2f\n",
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable ? "True" : "False",
        test_expiration_date,
        test_daily_consumption_rate
    );

    rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    char *updated_name = "UPDATED FOOD";
    int updated_quantity = 200;
    printf("Updating the name of the food batch to %s and quantity to %d\n", updated_name, updated_quantity);
    rc = foodbatch_db_update(
        &test_foodbatch_db,
        test_batch_id,
        updated_name,
        updated_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    assert(rc == SQLITE_OK);
    printf("Update operation was successful.\n");

    printf("Retrieving the food batch to test the changes.\n");

    struct foodbatch test_foodbatch = { 0 };
    foodbatch_db_get_by_batchid(&test_foodbatch_db, test_batch_id, &test_foodbatch);

    assert(strcmp(test_foodbatch.name, updated_name) == 0);
    assert(test_foodbatch.quantity == updated_quantity);
    printf("Retrieved food batch has the updated values.\n");

    teardown_cleanup();

    printf("Food batch database update test passed successfully.\n");
}

void test_foodbatch_db_check_batchid_exists() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

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

    printf(
        "Inserting a food batch with the following values:\n"
        "Batch ID: %d\n"
        "Name: %s\n"
        "Quantity: %d\n"
        "Is Perishable: %s\n"
        "Expiration Date: %s\n"
        "Daily Consumption Rate: %.2f\n",
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable ? "True" : "False",
        test_expiration_date,
        test_daily_consumption_rate
    );

    foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    printf("Attempting to check if the inserted food batch exists.\n");

    exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, test_batch_id);

    assert(exists == true);
    printf("Exists.\n");

    teardown_cleanup();

    printf("Food batch database check batch ID exists test passed successfully.\n");
}

void test_foodbatch_db_delete_by_id() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    int test_batch_id = 1;
    char *test_name = "Test Food";
    int test_quantity = 100;
    bool test_is_perishable = true;
    char *test_expiration_date = "2023-12-31";
    float test_daily_consumption_rate = 5.0;

    printf(
        "Inserting a food batch with the following values:\n"
        "Batch ID: %d\n"
        "Name: %s\n"
        "Quantity: %d\n"
        "Is Perishable: %s\n"
        "Expiration Date: %s\n"
        "Daily Consumption Rate: %.2f\n",
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable ? "True" : "False",
        test_expiration_date,
        test_daily_consumption_rate
    );

    foodbatch_db_insert(
        &test_foodbatch_db,
        test_batch_id,
        test_name,
        test_quantity,
        test_is_perishable,
        test_expiration_date,
        test_daily_consumption_rate
    );

    printf("Attempting to delete the inserted food batch.\n");

    int rc = foodbatch_db_delete_by_id(&test_foodbatch_db, test_batch_id);

    assert(rc == SQLITE_OK);
    printf("Operation successful.\n");

    printf("Check if the removed food batch still exists.\n");

    bool exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, test_batch_id);

    assert(exists == false);
    printf("Operation successful.\n");

    printf("Attempting to delete a non-existent foodbatch.\n");

    rc = foodbatch_db_delete_by_id(&test_foodbatch_db, test_batch_id);

    assert(rc == SQLITE_NOTFOUND);

    printf("Foodbatch not found.\n");

    teardown_cleanup();

    printf("Food batch database delete test passed successfully.\n");
}

// TEST DB FOODBATCH END

// TEST DB USER START

void test_user_db_create_user() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    bool test_is_admin = false;
    bool test_reset_password = true;

    printf(
        "Attempting to create a user with the following values:\n"
        "Username: %s\n"
        "Is Admin: %s\n"
        "Reset Password: %s\n",
        test_username,
        test_is_admin ? "True" : "False",
        test_reset_password ? "True" : "False"
    );

    int rc = user_db_create_user(&test_user_db, test_username, test_is_admin, test_reset_password);
    assert(rc == SQLITE_OK);
    printf("User created successfully.\n");

    printf("Attempting to create the same user again.\n");
    rc = user_db_create_user(&test_user_db, test_username, test_is_admin, test_reset_password);
    assert(rc == SQLITE_CONSTRAINT);
    printf("Attempt to create duplicate user failed as expected.\n");

    teardown_cleanup();

    printf("User creation test passed successfully.\n");
}

void test_user_db_authenticate() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    const char *test_password = "password123";
    bool test_is_admin = false;

    // First create a user with a known password
    printf("Creating test user for authentication\n");
    user_db_create_user(&test_user_db, test_username, test_is_admin, true);

    // Set the password
    printf("Setting password for test user\n");
    int rc = user_db_update_password(&test_user_db, test_username, test_password);
    assert(rc == SQLITE_OK);

    printf("Testing authentication with correct credentials\n");
    enum auth_result result = user_db_authenticate(&test_user_db, test_username, test_password);
    assert(result == AUTH_SUCCESS);
    printf("Authentication succeeded with correct credentials.\n");

    printf("Testing authentication with incorrect password\n");
    result = user_db_authenticate(&test_user_db, test_username, "wrongpassword");
    assert(result == AUTH_FAILURE);
    printf("Authentication failed with wrong password as expected.\n");

    printf("Testing authentication with non-existent user\n");
    result = user_db_authenticate(&test_user_db, "nonexistentuser", "password");
    assert(result == AUTH_FAILURE);
    printf("Authentication failed for non-existent user as expected.\n");

    teardown_cleanup();

    printf("Authentication test passed successfully.\n");
}

void test_user_db_get_by_username() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    bool test_is_admin = false;
    bool test_reset_password = true;

    printf("Creating test user for retrieval\n");
    int rc = user_db_create_user(&test_user_db, test_username, test_is_admin, test_reset_password);
    assert(rc == SQLITE_OK);

    struct user retrieved_user = { 0 };
    printf("Attempting to retrieve user by username\n");
    rc = user_db_get_by_username(&test_user_db, test_username, &retrieved_user);

    assert(rc == SQLITE_OK);
    assert(strcmp(retrieved_user.username, test_username) == 0);
    assert(retrieved_user.is_admin == test_is_admin);
    // Don't check reset_password as it's not returned by user_db_get_by_username
    printf("User retrieved successfully with matching data.\n");

    printf("Attempting to retrieve non-existent user\n");
    rc = user_db_get_by_username(&test_user_db, "nonexistentuser", &retrieved_user);
    assert(rc == SQLITE_NOTFOUND);
    printf("Non-existent user retrieval failed as expected.\n");

    teardown_cleanup();

    printf("User retrieval test passed successfully.\n");
}

void test_user_db_update_password() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    const char *old_password = "oldpassword";
    const char *new_password = "newpassword";
    bool test_is_admin = false;

    printf("Creating test user for password update\n");
    user_db_create_user(&test_user_db, test_username, test_is_admin, true);

    // Set initial password
    printf("Setting initial password\n");
    int rc = user_db_update_password(&test_user_db, test_username, old_password);
    assert(rc == SQLITE_OK);

    // Verify initial password works
    printf("Verifying initial password\n");
    enum auth_result result = user_db_authenticate(&test_user_db, test_username, old_password);
    assert(result == AUTH_SUCCESS);

    // Update password
    printf("Updating password\n");
    rc = user_db_update_password(&test_user_db, test_username, new_password);
    assert(rc == SQLITE_OK);

    // Verify old password no longer works
    printf("Verifying old password no longer works\n");
    result = user_db_authenticate(&test_user_db, test_username, old_password);
    assert(result == AUTH_FAILURE);

    // Verify new password works
    printf("Verifying new password works\n");
    result = user_db_authenticate(&test_user_db, test_username, new_password);
    assert(result == AUTH_SUCCESS);

    teardown_cleanup();

    printf("Password update test passed successfully.\n");
}

void test_user_db_update_admin_status() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    bool initial_admin_status = false;
    bool new_admin_status = true;

    printf("Creating test user for admin status update\n");
    int rc = user_db_create_user(&test_user_db, test_username, initial_admin_status, true);
    assert(rc == SQLITE_OK);

    // Get user ID
    struct user test_user = { 0 };
    rc = user_db_get_by_username(&test_user_db, test_username, &test_user);
    assert(rc == SQLITE_OK);

    printf("Verifying initial admin status\n");
    assert(test_user.is_admin == initial_admin_status);

    printf("Updating admin status\n");
    rc = user_db_update_admin_status(&test_user_db, test_user.user_id, new_admin_status);
    assert(rc == SQLITE_OK);

    printf("Verifying updated admin status\n");
    rc = user_db_get_by_username(&test_user_db, test_username, &test_user);
    assert(rc == SQLITE_OK);
    assert(test_user.is_admin == new_admin_status);

    teardown_cleanup();

    printf("Admin status update test passed successfully.\n");
}

void test_user_db_delete() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    const char *test_username = "testuser";
    bool test_is_admin = false;

    printf("Creating test user for deletion\n");
    int rc = user_db_create_user(&test_user_db, test_username, test_is_admin, true);
    assert(rc == SQLITE_OK);

    // Get user ID
    struct user test_user = { 0 };
    rc = user_db_get_by_username(&test_user_db, test_username, &test_user);
    assert(rc == SQLITE_OK);

    printf("Verifying user exists before deletion\n");
    bool exists = user_db_user_exists(&test_user_db, test_username);
    assert(exists == true);

    printf("Deleting user\n");
    rc = user_db_delete(&test_user_db, test_user.user_id);
    assert(rc == SQLITE_OK);

    printf("Verifying user no longer exists\n");
    exists = user_db_user_exists(&test_user_db, test_username);
    assert(exists == false);

    teardown_cleanup();

    printf("User deletion test passed successfully.\n");
}

void test_user_db_admin_creation() {
    const char *test_user_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_user_filename, user_db_create_table);

    setup_cleanup(test_user_filename, &test_user_db);

    printf("Verifying admin user exists\n");
    bool exists = user_db_user_exists(&test_user_db, "admin");
    assert(exists == true);

    printf("Attempting to authenticate with default admin credentials\n");
    enum auth_result result = user_db_authenticate(&test_user_db, "admin", "admin");
    assert(result == AUTH_SUCCESS);
    printf("Admin doesn't require password reset as expected.\n");

    teardown_cleanup();

    printf("Admin creation test passed successfully.\n");
}

// TEST DB USER END

int main() {
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

    test_user_db_create_user();
    test_user_db_authenticate();
    test_user_db_get_by_username();
    test_user_db_update_password();
    test_user_db_update_admin_status();
    test_user_db_delete();
    test_user_db_admin_creation();

    return 0;
}
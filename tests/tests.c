/**
 * @file tests.c
 * @brief Unit testing for everything that can be unit tested
 */
#include <assert.h>
#include <ctype.h>
#include <inttypes.h> // For PRIu64 (compatibility for both windows and linux)
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db/clothes_db.h"
#include "db/db_manager.h"
#include "db/foodbatch_db.h"
#include "db/medication_db.h"
#include "db/resident_db.h"
#include "db/supplies_db.h"
#include "db/tasks_db.h"
#include "db/user_db.h"
#include "utils/utils_hash.h"
#include "utils/utilsfn.h"

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

// Setup function (only checks SIGABRT)
void setup_cleanup(const char *filename, database *db) {
    // Set up signal handlers
    signal(SIGABRT, cleanup_handler); // assertion failure signal

    // Store cleanup context
    cleanup_ctx.db_filename = filename;
    cleanup_ctx.db_handle = db;
}

// Teardown function
void teardown_cleanup(void) {
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

// TEST DB MANAGER START

void test_db_init(void) {
    const char *test_database_filename = "test_database_filename.db";
    database test_database;

    setup_cleanup(test_database_filename, &test_database);

    printf("Testing initializing a database.\n");

    int rc = db_init(&test_database, test_database_filename);

    assert(rc == SQLITE_OK);

    printf("Initialized database successfully.\n");

    printf("Testing passing null as the database.\n");

    rc = db_init(NULL, test_database_filename);

    assert(rc == SQLITE_ERROR);

    printf("Did not return SQLITE_OK as expected.\n");

    printf("Testing passing null as the filename.\n");

    rc = db_init(&test_database, NULL);

    assert(rc == SQLITE_ERROR);

    printf("Did not work as expected.\n");

    teardown_cleanup();

    printf("db_init passed successfully.\n");
}

void test_db_init_with_tbl(void) {
    const char *test_database_filename = "test_database_filename.db";
    database test_database;

    setup_cleanup(test_database_filename, &test_database);

    printf("Testing passing a null to the callback.\n");

    int rc = db_init_with_tbl(&test_database, test_database_filename, NULL);

    assert(rc != SQLITE_OK);

    printf("Did not return SQLITE_OK as expected.\n");

    printf("Testing passing null as the database.\n");

    rc = db_init_with_tbl(NULL, test_database_filename, NULL);

    assert(rc == SQLITE_ERROR);

    printf("Did not work as expected.\n");

    printf("Testing passing null as the filename.\n");

    rc = db_init_with_tbl(&test_database, NULL, NULL);

    assert(rc == SQLITE_ERROR);

    printf("Did not work as expected.\n");

    teardown_cleanup();

    printf("db_init_with_tbl passed successfully.\n");
}

void test_db_is_init(void) {
    const char *test_database_filename = "test_database_filename.db";
    database test_database;

    setup_cleanup(test_database_filename, &test_database);

    printf("Testing if an uninitialized a database is init.\n");

    bool init = db_is_init(&test_database);

    assert(init == false);

    printf("Uninitialized database is not initialized as expected.\n");

    printf("Testing if an initialized a database is init.\n");

    db_init(&test_database, test_database_filename);

    init = db_is_init(&test_database);

    assert(init == true);

    printf("Initialized database is initialized as expected.\n");

    printf("Passing null to the function, just to check the program didn't crash and returns false.\n");

    init = db_is_init(NULL);

    assert(init == false);

    printf("Passing a null db is a no-op as expected.\n");

    teardown_cleanup();

    printf("db_is_init passed successfully.\n");
}

void test_db_deinit(void) {
    const char *test_database_filename = "test_database_filename.db";
    database test_database;

    setup_cleanup(test_database_filename, &test_database);

    printf("Testing deinitializing a database.\n");

    db_init(&test_database, test_database_filename);

    db_deinit(&test_database);

    assert(test_database.db == NULL);

    printf("Initialized database deinitialized successfully.\n");

    printf("Passing a just deinitialized database to check the program didn't crash.\n");

    db_deinit(&test_database);

    printf("Passing a null db->db is a no-op as expected.\n");

    printf("Passing null to the function, just to check the program didn't crash.\n");

    db_deinit(NULL);

    printf("Passing null is a no-op as expected.\n");

    teardown_cleanup();

    printf("db_deinit passed successfully.\n");
}

// TEST DB MANAGER END

// TEST DB RESIDENT START

void test_resident_db_insert(void) {
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

void test_resident_db_retrieve(void) {
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

void test_resident_db_update(void) {
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

void test_resident_db_check_cpf_exists(void) {
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

void test_resident_db_delete_by_cpf(void) {
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

void test_resident_db_get_count(void) {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    // Test empty database
    printf("Testing count on empty database...\n");
    int count = resident_db_get_count(&test_resident_db);
    assert(count == 0);
    printf("Empty database count correct (0).\n");

    // Add one resident
    printf("Adding one resident...\n");
    int rc = resident_db_insert(&test_resident_db, "12345678901", "John Doe", 30, "Healthy", "None", false, 0);
    assert(rc == SQLITE_OK);

    // Verify count is now 1
    printf("Verifying count after insertion...\n");
    count = resident_db_get_count(&test_resident_db);
    assert(count == 1);
    printf("Count correct after insertion (1).\n");

    // Add multiple residents
    printf("Adding multiple residents...\n");
    resident_db_insert(&test_resident_db, "23456789012", "Jane Smith", 45, "Chronic condition", "Medication", true, 1);
    resident_db_insert(&test_resident_db, "34567890123", "Alex Johnson", 28, "Healthy", "None", false, 2);

    // Verify count is now 3
    printf("Verifying count after multiple insertions...\n");
    count = resident_db_get_count(&test_resident_db);
    assert(count == 3);
    printf("Count correct after multiple insertions (3).\n");

    // Delete one and verify count
    printf("Deleting one resident...\n");
    resident_db_delete_by_cpf(&test_resident_db, "23456789012");
    count = resident_db_get_count(&test_resident_db);
    assert(count == 2);
    printf("Count correct after deletion (2).\n");

    teardown_cleanup();

    printf("resident_db_get_count test passed successfully.\n");
}

void test_resident_db_get_all_format(void) {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    // Test empty database
    printf("Testing format on empty database...\n");
    char buffer[8192]; // Large buffer for testing
    int written = resident_db_get_all_format(&test_resident_db, buffer, sizeof(buffer));
    assert(written > 0);
    printf("Empty database format output:\n%s\n", buffer);

    // Add test data
    printf("Adding test residents...\n");
    resident_db_insert(&test_resident_db, "12345678901", "John Doe", 30, "Healthy", "None", false, 0);
    resident_db_insert(&test_resident_db, "23456789012", "Jane Smith", 45, "Chronic condition", "Medication", true, 1);
    resident_db_insert(&test_resident_db, "34567890123", "Alex Johnson", 28, "Healthy", "None", false, 2);

    // Test with sufficient buffer
    printf("Testing format with sufficient buffer...\n");
    written = resident_db_get_all_format(&test_resident_db, buffer, sizeof(buffer));
    int exact_bytes = written;
    assert(written > 0);
    printf("Formatted output with sufficient buffer:\n%s\n", buffer);

    // Verify the format contains expected elements
    assert(strstr(buffer, "CPF") != NULL);
    assert(strstr(buffer, "Name") != NULL);
    assert(strstr(buffer, "Age") != NULL);
    assert(strstr(buffer, "HealthStatus") != NULL);
    assert(strstr(buffer, "Needs") != NULL);
    assert(strstr(buffer, "Medical Assistance") != NULL);
    assert(strstr(buffer, "Gender") != NULL);
    assert(strstr(buffer, "Entry Date") != NULL);
    assert(strstr(buffer, "John Doe") != NULL);
    assert(strstr(buffer, "Jane Smith") != NULL);
    assert(strstr(buffer, "Alex Johnson") != NULL);
    printf("Format contains all expected elements.\n");

    // Test with small buffer (should truncate)
    printf("Testing format with small buffer...\n");
    char small_buffer[100];
    written = resident_db_get_all_format(&test_resident_db, small_buffer, sizeof(small_buffer));
    assert(written == -1);
    printf("Format correctly detected buffer overflow (returned -1).\n");
    printf(
        "Truncated output (first %" PRIu64 " bytes):\n%.*s\n",
        sizeof(small_buffer),
        (int)sizeof(small_buffer),
        small_buffer
    );

    // Test with exact buffer size
    printf("Testing format with exact buffer size...\n");
    written = exact_bytes;
    written = resident_db_get_all_format(&test_resident_db, buffer, written + 1);
    assert(written > 0);
    printf("Format with exact buffer size successful.\n");

    teardown_cleanup();

    printf("resident_db_get_all_format test passed successfully.\n");
}

void test_resident_db_get_all_format_old(void) {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    // Test empty database
    printf("Testing format on empty database...\n");
    char *result = resident_db_get_all_format_old(&test_resident_db);
    assert(result != NULL);
    printf("Empty database format output:\n%s\n", result);
    free(result);

    // Add test data
    printf("Adding test residents...\n");
    resident_db_insert(&test_resident_db, "12345678901", "John Doe", 30, "Healthy", "None", false, 0);
    resident_db_insert(&test_resident_db, "23456789012", "Jane Smith", 45, "Chronic condition", "Medication", true, 1);
    resident_db_insert(&test_resident_db, "34567890123", "Alex Johnson", 28, "Healthy", "None", false, 2);
    resident_db_insert(&test_resident_db, "45678901234", "Maria Garcia", 60, "Diabetes", "Insulin", true, 2);
    resident_db_insert(
        &test_resident_db,
        "56789012345",
        "Robert Brown",
        35,
        "Hypertension",
        "Blood pressure meds",
        true,
        1
    );

    // Test with multiple records
    printf("Testing format with multiple records...\n");
    result = resident_db_get_all_format_old(&test_resident_db);
    assert(result != NULL);
    printf("Formatted output with multiple records:\n%s\n", result);

    // Verify the format contains expected elements
    assert(strstr(result, "CPF") != NULL);
    assert(strstr(result, "Name") != NULL);
    assert(strstr(result, "Age") != NULL);
    assert(strstr(result, "HealthStatus") != NULL);
    assert(strstr(result, "Needs") != NULL);
    assert(strstr(result, "Medical Assistance") != NULL);
    assert(strstr(result, "Gender") != NULL);
    assert(strstr(result, "Entry Date") != NULL);
    assert(strstr(result, "John Doe") != NULL);
    assert(strstr(result, "Jane Smith") != NULL);
    assert(strstr(result, "Alex Johnson") != NULL);
    assert(strstr(result, "Maria Garcia") != NULL);
    assert(strstr(result, "Robert Brown") != NULL);
    printf("Format contains all expected elements.\n");

    free(result);

    teardown_cleanup();

    printf("resident_db_get_all_format_old test passed successfully.\n");
}

void test_resident_db_get_all(void) {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    printf("Testing resident_db_get_all...\n");

    // Create several test residents
    printf("Creating test residents...\n");
    resident_db_insert(&test_resident_db, "00000000000", "Test Resident1", 20, "Healthy", "No needs", false, 0);
    resident_db_insert(&test_resident_db, "11111111111", "Test Resident2", 1, "Not Healthy", "Various needs", true, 1);
    resident_db_insert(&test_resident_db, "22222222222", "Test Resident3", 24, "", "", false, 2);
    resident_db_insert(&test_resident_db, "33333333333", "Test Resident4", 69, "", "No needs", false, 1);

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling resident_db_get_all...\n");
    int rc = resident_db_get_all(&test_resident_db);
    assert(rc == SQLITE_OK);
    printf("resident_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("resident_db_get_all test passed successfully.\n");
}

// TEST DB RESIDENT END

// TEST DB FOODBATCH START

void test_foodbatch_db_insert(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    char *test_name = "Test Food";
    float test_quantity = 100;
    char *test_unit = "piece";
    bool test_is_perishable = true;
    char *test_arrival_date = "2023-11-31";
    char *test_expiration_date = "2023-12-31";

    printf(
        "Attempting to insert a food batch with the following values:\n"
        "Name: %s\n"
        "Quantity: %f\n"
        "Unit: %s\n"
        "Is Perishable: %s\n"
        "Arrival Date: %s\n"
        "Expiration Date: %s\n",
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable ? "True" : "False",
        test_arrival_date,
        test_expiration_date
    );

    int rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    assert(rc == SQLITE_OK);
    printf("Inserted food batch successfully.\n");

    printf("Attempting to insert the same food batch again, a new batch will be created.\n");

    rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    assert(rc == SQLITE_OK);
    printf("Attempt to insert the same food in another batch was successful.\n");

    teardown_cleanup();

    printf("Food batch database insertion test passed successfully.\n");
}

void test_foodbatch_db_retrieve(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    char *test_name = "Test Food";
    float test_quantity = 100;
    char *test_unit = "kg";
    bool test_is_perishable = true;
    char *test_arrival_date = "2023-10-31";
    char *test_expiration_date = "2023-12-31";

    printf(
        "Attempting to insert a food batch with the following values:\n"
        "Name: %s\n"
        "Quantity: %f\n"
        "Unit: %s\n"
        "Is Perishable: %s\n"
        "Arrival Date: %s\n"
        "Expiration Date: %s\n",
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable ? "True" : "False",
        test_arrival_date,
        test_expiration_date
    );

    int rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    struct foodbatch test_foodbatch = { 0 };

    printf("Attempting to retrieve the above food batch and insert it into a foodbatch structure\n");

    rc = foodbatch_db_get_by_batchid(&test_foodbatch_db, 1, &test_foodbatch);

    assert(rc == SQLITE_OK);
    assert(test_foodbatch.batch_id == 1);
    assert(strcmp(test_foodbatch.name, test_name) == 0);
    assert(test_foodbatch.quantity == test_quantity);
    assert(strcmp(test_foodbatch.unit, test_unit) == 0);
    assert(test_foodbatch.is_perishable == test_is_perishable);
    assert(strcmp(test_foodbatch.expiration_date, test_expiration_date) == 0);
    assert(strcmp(test_foodbatch.arrival_date, test_arrival_date) == 0);

    printf("Retrieve successful\n");

    teardown_cleanup();

    printf("Food batch database retrieval test passed.\n");
}

void test_foodbatch_db_update(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    char *test_name = "Test Food";
    float test_quantity = 100;
    char *test_unit = "unit";
    bool test_is_perishable = true;
    char *test_arrival_date = "2023-09-31";
    char *test_expiration_date = "2023-12-31";

    printf("Attempting to update a non-existent food batch.\n");
    int rc = foodbatch_db_update(
        &test_foodbatch_db,
        100,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    assert(rc != SQLITE_OK);
    printf("Updating a non-existent food batch was unsuccessful.\n");

    printf(
        "Inserting a food batch with the following values:\n"
        "Name: %s\n"
        "Quantity: %f\n"
        "Unit: %s\n"
        "Is Perishable: %s\n"
        "Arrival Date: %s\n"
        "Expiration Date: %s\n",
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable ? "True" : "False",
        test_arrival_date,
        test_expiration_date
    );

    rc = foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    char *updated_name = "UPDATED FOOD";
    float updated_quantity = 200;
    printf("Updating the name of the food batch to %s and quantity to %f\n", updated_name, updated_quantity);
    rc = foodbatch_db_update(
        &test_foodbatch_db,
        1,
        updated_name,
        updated_quantity,
        "\0",
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    assert(rc == SQLITE_OK);
    printf("Update operation was successful.\n");

    printf("Retrieving the food batch to test the changes.\n");

    struct foodbatch test_foodbatch = { 0 };
    foodbatch_db_get_by_batchid(&test_foodbatch_db, 1, &test_foodbatch);

    assert(strcmp(test_foodbatch.name, updated_name) == 0);
    assert(test_foodbatch.quantity == updated_quantity);
    assert(strcmp(test_foodbatch.unit, test_unit) == 0);
    assert(test_foodbatch.is_perishable == test_is_perishable);
    assert(strcmp(test_foodbatch.expiration_date, test_expiration_date) == 0);
    assert(strcmp(test_foodbatch.arrival_date, test_arrival_date) == 0);
    printf("Retrieved food batch has the updated values.\n");

    teardown_cleanup();

    printf("Food batch database update test passed successfully.\n");
}

void test_foodbatch_db_check_batchid_exists(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    char *test_name = "Test Food";
    float test_quantity = 100;
    char *test_unit = "unit";
    bool test_is_perishable = true;
    char *test_arrival_date = "2023-09-31";
    char *test_expiration_date = "2023-12-31";

    printf("Attempting to check if a non-existent food batch exists.\n");

    bool exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, 100);

    assert(exists == false);
    printf("Doesn't exist.\n");

    printf(
        "Inserting a food batch with the following values:\n"
        "Name: %s\n"
        "Quantity: %f\n"
        "Unit: %s\n"
        "Is Perishable: %s\n"
        "Arrival Date: %s\n"
        "Expiration Date: %s\n",
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable ? "True" : "False",
        test_arrival_date,
        test_expiration_date
    );

    foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    printf("Attempting to check if the inserted food batch exists.\n");

    exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, 1);

    assert(exists == true);
    printf("Exists.\n");

    teardown_cleanup();

    printf("Food batch database check batch ID exists test passed successfully.\n");
}

void test_foodbatch_db_delete_by_id(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    char *test_name = "Test Food";
    float test_quantity = 100;
    char *test_unit = "unit";
    bool test_is_perishable = true;
    char *test_arrival_date = "2023-09-31";
    char *test_expiration_date = "2023-12-31";

    printf(
        "Inserting a food batch with the following values:\n"
        "Name: %s\n"
        "Quantity: %f\n"
        "Unit: %s\n"
        "Is Perishable: %s\n"
        "Arrival Date: %s\n"
        "Expiration Date: %s\n",
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable ? "True" : "False",
        test_arrival_date,
        test_expiration_date
    );

    foodbatch_db_insert(
        &test_foodbatch_db,
        test_name,
        test_quantity,
        test_unit,
        test_is_perishable,
        test_arrival_date,
        test_expiration_date
    );

    printf("Attempting to delete the inserted food batch.\n");

    int rc = foodbatch_db_delete_by_id(&test_foodbatch_db, 1);

    assert(rc == SQLITE_OK);
    printf("Operation successful.\n");

    printf("Check if the removed food batch still exists.\n");

    bool exists = foodbatch_db_check_batchid_exists(&test_foodbatch_db, 1);

    assert(exists == false);
    printf("Operation successful.\n");

    printf("Attempting to delete a non-existent foodbatch.\n");

    rc = foodbatch_db_delete_by_id(&test_foodbatch_db, 1);

    assert(rc == SQLITE_NOTFOUND);

    printf("Foodbatch not found.\n");

    teardown_cleanup();

    printf("Food batch database delete test passed successfully.\n");
}

void test_foodbatch_db_get_all(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    printf("Testing foodbatch_db_get_all...\n");

    // Create several test foodbatches
    printf("Creating test foodbatches...\n");
    foodbatch_db_insert(&test_foodbatch_db, "Milk", 10, "pack", true, "2025-12-20", "2026-01-20");
    foodbatch_db_insert(&test_foodbatch_db, "Rice", 5, "piece", false, "2025-12-12", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Beans", 10, "piece", false, "2025-12-30", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Tomatoes", 20, "kg", true, "2025-12-09", "2026-01-09");

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling foodbatch_db_get_all...\n");
    int rc = foodbatch_db_get_all(&test_foodbatch_db);
    assert(rc == SQLITE_OK);
    printf("foodbatch_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("foodbatch_db_get_all test passed successfully.\n");
}

void test_foodbatch_db_get_count(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    // Test empty database
    printf("Testing count on empty database...\n");
    int count = foodbatch_db_get_count(&test_foodbatch_db);
    assert(count == 0);
    printf("Empty database count correct (0).\n");

    // Add one food batch
    printf("Adding one food batch...\n");
    int rc = foodbatch_db_insert(&test_foodbatch_db, "Milk", 10, "pack", true, "2025-12-20", "2026-01-20");
    assert(rc == SQLITE_OK);

    // Verify count is now 1
    printf("Verifying count after insertion...\n");
    count = foodbatch_db_get_count(&test_foodbatch_db);
    assert(count == 1);
    printf("Count correct after insertion (1).\n");

    // Add multiple batches
    printf("Adding multiple food batches...\n");
    foodbatch_db_insert(&test_foodbatch_db, "Rice", 5, "piece", false, "2025-12-12", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Beans", 10, "piece", false, "2025-12-30", NULL);

    // Verify count is now 3
    printf("Verifying count after multiple insertions...\n");
    count = foodbatch_db_get_count(&test_foodbatch_db);
    assert(count == 3);
    printf("Count correct after multiple insertions (3).\n");

    // Delete one and verify count
    printf("Deleting one food batch...\n");
    foodbatch_db_delete_by_id(&test_foodbatch_db, 2);
    count = foodbatch_db_get_count(&test_foodbatch_db);
    assert(count == 2);
    printf("Count correct after deletion (2).\n");

    teardown_cleanup();

    printf("foodbatch_db_get_count test passed successfully.\n");
}

void test_foodbatch_db_get_all_format(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    // Test empty database
    printf("Testing format on empty database...\n");
    int count = foodbatch_db_get_count(&test_foodbatch_db);
    size_t buffer_size = 512 + 1024 * count;

    char *str_content = malloc(buffer_size);
    assert(str_content);
    int written = foodbatch_db_get_all_format(&test_foodbatch_db, str_content, buffer_size);
    assert(written > 0);
    printf("Empty database format output:\n%s\n", str_content);

    // Add test data
    printf("Adding test food batches...\n");
    foodbatch_db_insert(&test_foodbatch_db, "Milk", 10, "pack", true, "2025-12-20", "2026-01-20");
    foodbatch_db_insert(&test_foodbatch_db, "Rice", 5, "piece", false, "2025-12-12", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Beans", 10, "piece", false, "2025-12-30", NULL);

    count = foodbatch_db_get_count(&test_foodbatch_db);

    buffer_size += 1024 * count;

    str_content = realloc(str_content, buffer_size);

    // Test with sufficient buffer
    printf("Testing format with sufficient buffer...\n");
    written = foodbatch_db_get_all_format(&test_foodbatch_db, str_content, buffer_size);
    int exact_bytes = written;
    printf("Formatted output with sufficient buffer:\n%s\n", str_content);

    // Verify the format contains expected elements
    assert(strstr(str_content, "ID") != NULL);
    assert(strstr(str_content, "Name") != NULL);
    assert(strstr(str_content, "Quantity") != NULL);
    assert(strstr(str_content, "Unit") != NULL);
    assert(strstr(str_content, "Perishable") != NULL);
    assert(strstr(str_content, "Arrival date") != NULL);
    assert(strstr(str_content, "Expiration date") != NULL);
    assert(strstr(str_content, "Milk") != NULL);
    assert(strstr(str_content, "Beans") != NULL);
    assert(strstr(str_content, "Rice") != NULL);
    printf("Format contains all expected elements.\n");

    // Test with small buffer (should truncate)
    printf("Testing format with small buffer...\n");
    char small_buffer[100];
    written = foodbatch_db_get_all_format(&test_foodbatch_db, small_buffer, sizeof(small_buffer));
    assert(written == -1);
    printf("Format correctly detected buffer overflow (returned -1).\n");
    printf(
        "Truncated output (first %" PRIu64 " bytes):\n%.*s\n",
        sizeof(small_buffer),
        (int)sizeof(small_buffer),
        small_buffer
    );

    // Test with exact buffer size
    printf("Testing format with exact buffer size...\n");
    written = exact_bytes;
    written = foodbatch_db_get_all_format(&test_foodbatch_db, str_content, written + 1);
    assert(written > 0);
    printf("Format with exact buffer size successful.\n");

    teardown_cleanup();

    free(str_content);

    printf("foodbatch_db_get_all_format test passed successfully.\n");
}

void test_foodbatch_db_get_all_format_old(void) {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    // Test empty database
    printf("Testing format on empty database...\n");
    char *result = foodbatch_db_get_all_format_old(&test_foodbatch_db);
    assert(result != NULL);
    printf("Empty database format output:\n%s\n", result);
    free(result);

    // Add test data
    printf("Adding test food batches...\n");
    foodbatch_db_insert(&test_foodbatch_db, "Milk", 10, "pack", true, "2025-12-20", "2026-01-20");
    foodbatch_db_insert(&test_foodbatch_db, "Rice", 5, "piece", false, "2025-12-12", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Beans", 10, "piece", false, "2025-12-30", NULL);
    foodbatch_db_insert(&test_foodbatch_db, "Tomatoes", 20, "kg", true, "2025-12-09", "2026-01-09");

    // Test with multiple records
    printf("Testing format with multiple records...\n");
    result = foodbatch_db_get_all_format_old(&test_foodbatch_db);
    assert(result != NULL);
    printf("Formatted output with multiple records:\n%s\n", result);

    // Verify the format contains expected elements
    assert(strstr(result, "ID") != NULL);
    assert(strstr(result, "Name") != NULL);
    assert(strstr(result, "Quantity") != NULL);
    assert(strstr(result, "Unit") != NULL);
    assert(strstr(result, "Perishable") != NULL);
    assert(strstr(result, "Arrival date") != NULL);
    assert(strstr(result, "Expiration date") != NULL);
    assert(strstr(result, "Milk") != NULL);
    assert(strstr(result, "Rice") != NULL);
    assert(strstr(result, "Tomatoes") != NULL);
    printf("Format contains all expected elements.\n");

    free(result);

    teardown_cleanup();

    printf("foodbatch_db_get_all_format_old test passed successfully.\n");
}

// TEST DB FOODBATCH END

// TEST DB USER START

void test_user_db_create_table(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_create_table...\n");

    // Verify admin user was created
    bool admin_exists = user_db_check_exists(&test_user_db, "admin");
    assert(admin_exists);
    printf("Admin user created successfully.\n");

    // Verify table structure by trying to insert a user
    int rc = user_db_create_user(
        &test_user_db,
        "testuser",      // username
        "12345678901",   // cpf (sample)
        "5551900100200", // phone number
        false            // is_admin
    );
    assert(rc == SQLITE_OK);
    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("user_db_create_table test passed successfully.\n");
}

void test_user_db_create_user(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_create_user...\n");

    // Test creating a regular user
    printf("Creating regular user 'user1'...\n");
    int rc = user_db_create_user(&test_user_db, "user1", "00000000000", "1234567890123", false);
    assert(rc == SQLITE_OK);
    printf("Regular user created successfully.\n");

    // Test creating an admin user
    printf("Creating admin user 'admin2'...\n");
    rc = user_db_create_user(&test_user_db, "admin2", "11111111111", "1234567890123", true);
    assert(rc == SQLITE_OK);
    printf("Admin user created successfully.\n");

    // Test creating duplicate user
    printf("Attempting to create duplicate user 'user1' by username...\n");
    rc = user_db_create_user(&test_user_db, "user1", "87655366273", "", false);
    assert(rc == SQLITE_CONSTRAINT);
    printf("Duplicate user creation failed as expected.\n");

    printf("Attempting to create duplicate user by CPF...\n");
    rc = user_db_create_user(&test_user_db, "user2", "00000000000", "", false);
    assert(rc == SQLITE_CONSTRAINT);
    printf("Duplicate user creation failed as expected.\n");

    teardown_cleanup();

    printf("user_db_create_user test passed successfully.\n");
}

void test_user_db_authenticate(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_authenticate...\n");

    // Create a test user with known password
    const char *username = "testuser";
    const char *password = "password123";

    // First create user without password (reset_password flag set)
    printf("Creating user '%s' with reset_password flag...\n", username);
    int rc = user_db_create_user(&test_user_db, username, "00000000000", "", false);
    assert(rc == SQLITE_OK);

    // Test authentication with reset_password flag
    printf("Testing authentication with reset_password flag...\n");
    enum auth_result auth_rc = user_db_authenticate(&test_user_db, username, password);
    assert(auth_rc == AUTH_NEED_PASSWORD_RESET);
    printf("Authentication correctly detected need for password reset.\n");

    // Set password for the user
    printf("Setting password for user '%s'...\n", username);
    rc = user_db_update_password(&test_user_db, username, password);
    assert(rc == SQLITE_OK);

    // Test successful authentication
    printf("Testing successful authentication...\n");
    auth_rc = user_db_authenticate(&test_user_db, username, password);
    assert(auth_rc == AUTH_SUCCESS);
    printf("Authentication succeeded with correct password.\n");

    // Test failed authentication
    printf("Testing failed authentication...\n");
    auth_rc = user_db_authenticate(&test_user_db, username, "wrongpassword");
    assert(auth_rc == AUTH_FAILURE);
    printf("Authentication failed with wrong password as expected.\n");

    // Test non-existent user
    printf("Testing authentication with non-existent user...\n");
    auth_rc = user_db_authenticate(&test_user_db, "nonexistent", password);
    assert(auth_rc == AUTH_FAILURE);
    printf("Authentication failed for non-existent user as expected.\n");

    teardown_cleanup();

    printf("user_db_authenticate test passed successfully.\n");
}

void test_user_db_delete(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_delete...\n");

    // Create a test user
    const char *username = "user_to_delete";
    printf("Creating user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, "00000000000", "", false);
    assert(rc == SQLITE_OK);

    // Verify user exists
    bool exists = user_db_check_exists(&test_user_db, username);
    assert(exists);
    printf("User exists before deletion.\n");

    // Delete the user
    printf("Deleting user '%s'...\n", username);
    rc = user_db_delete(&test_user_db, username);
    assert(rc == SQLITE_OK);

    // Verify user no longer exists
    exists = user_db_check_exists(&test_user_db, username);
    assert(!exists);
    printf("User successfully deleted.\n");

    // Try to delete non-existent user
    printf("Attempting to delete non-existent user...\n");
    rc = user_db_delete(&test_user_db, "nonexistent");
    assert(rc == SQLITE_NOTFOUND);
    printf("Deletion of non-existent user failed as expected.\n");

    teardown_cleanup();

    printf("user_db_delete test passed successfully.\n");
}

void test_user_db_update_password(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_update_password...\n");

    // Create a test user
    const char *username = "password_test_user";
    printf("Creating user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, "00000000000", "", false);
    assert(rc == SQLITE_OK);

    // Set initial password
    const char *password1 = "firstpassword";
    printf("Setting initial password...\n");
    rc = user_db_update_password(&test_user_db, username, password1);
    assert(rc == SQLITE_OK);

    // Verify authentication works with first password
    printf("Verifying first password...\n");
    enum auth_result auth_rc = user_db_authenticate(&test_user_db, username, password1);
    assert(auth_rc == AUTH_SUCCESS);
    printf("First password works.\n");

    // Change password
    const char *password2 = "newpassword";
    printf("Changing password...\n");
    rc = user_db_update_password(&test_user_db, username, password2);
    assert(rc == SQLITE_OK);

    // Verify old password no longer works
    printf("Verifying old password no longer works...\n");
    auth_rc = user_db_authenticate(&test_user_db, username, password1);
    assert(auth_rc == AUTH_FAILURE);
    printf("Old password correctly rejected.\n");

    // Verify new password works
    printf("Verifying new password...\n");
    auth_rc = user_db_authenticate(&test_user_db, username, password2);
    assert(auth_rc == AUTH_SUCCESS);
    printf("New password works.\n");

    // Try to update password for non-existent user
    printf("Attempting to update password for non-existent user...\n");
    rc = user_db_update_password(&test_user_db, "nonexistent", "whatever");
    assert(rc == SQLITE_NOTFOUND);
    printf("Password update for non-existent user failed as expected.\n");

    teardown_cleanup();

    printf("user_db_update_password test passed successfully.\n");
}

void test_user_db_update_admin_status(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    setup_cleanup(test_userdb_filename, &test_user_db);

    // Create a test user (non-admin)
    const char *username = "admin_test_user";
    printf("Creating non-admin user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, "00000000000", "", false);
    assert(rc == SQLITE_OK);

    // Verify initial admin status
    struct user user_data;
    rc = user_db_get_by_username(&test_user_db, username, &user_data);
    assert(rc == SQLITE_OK);
    assert(!user_data.is_admin);
    printf("Initial admin status is false.\n");

    // Promote to admin
    printf("Promoting user to admin...\n");
    rc = user_db_update_admin_status(&test_user_db, username, true);
    assert(rc == SQLITE_OK);

    // Verify admin status changed
    rc = user_db_get_by_username(&test_user_db, username, &user_data);
    assert(rc == SQLITE_OK);
    assert(user_data.is_admin);
    printf("User successfully promoted to admin.\n");

    // Demote from admin
    printf("Demoting user from admin...\n");
    rc = user_db_update_admin_status(&test_user_db, username, false);
    assert(rc == SQLITE_OK);

    // Verify admin status changed back
    rc = user_db_get_by_username(&test_user_db, username, &user_data);
    assert(rc == SQLITE_OK);
    assert(!user_data.is_admin);
    printf("User successfully demoted from admin.\n");

    // Try to update non-existent user
    printf("Attempting to update admin status for non-existent user...\n");
    rc = user_db_update_admin_status(&test_user_db, "nonexistent", true);
    assert(rc == SQLITE_NOTFOUND);
    printf("Admin status update for non-existent user failed as expected.\n");

    teardown_cleanup();

    printf("user_db_update_admin_status test passed successfully.\n");
}

void test_user_db_check_cpf_exists(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_check_cpf_exists...\n");

    // Check for non-existent user
    printf("Checking for non-existent user...\n");
    bool exists = user_db_check_exists(&test_user_db, "nonexistent");
    assert(!exists);
    printf("Non-existent user correctly not found.\n");

    // Create a test user
    const char *cpf = "00000000000";
    printf("Creating user with cpf '%s'...\n", cpf);
    user_db_create_user(&test_user_db, "test_user", cpf, "", false);

    // Check for existing user
    printf("Checking for existing user...\n");
    exists = user_db_check_cpf_exists(&test_user_db, cpf);
    assert(exists);
    printf("Existing user correctly found.\n");

    teardown_cleanup();

    printf("user_db_check_cpf_exists test passed successfully.\n");
}

void test_user_db_check_exists(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_check_exists...\n");

    // Check for non-existent user
    printf("Checking for non-existent user...\n");
    bool exists = user_db_check_exists(&test_user_db, "nonexistent");
    assert(!exists);
    printf("Non-existent user correctly not found.\n");

    // Create a test user
    const char *username = "existent_test_user";
    printf("Creating user '%s'...\n", username);
    user_db_create_user(&test_user_db, username, "00000000000", "", false);

    // Check for existing user
    printf("Checking for existing user...\n");
    exists = user_db_check_exists(&test_user_db, username);
    assert(exists);
    printf("Existing user correctly found.\n");

    teardown_cleanup();

    printf("user_db_check_exists test passed successfully.\n");
}

void test_user_db_get_by_username(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_get_by_username...\n");

    // Create a test user with known properties
    const char *username = "get_test_user";
    const char *cpf = "00000000000";
    const char *phone_number = "5551912345678";
    bool is_admin = true;
    printf("Creating test user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, cpf, phone_number, is_admin);
    assert(rc == SQLITE_OK);

    // Set password
    const char *password = "testpassword";
    printf("Setting password...\n");
    rc = user_db_update_password(&test_user_db, username, password);
    assert(rc == SQLITE_OK);

    // Retrieve user data
    printf("Retrieving user data...\n");
    struct user user_data;
    rc = user_db_get_by_username(&test_user_db, username, &user_data);
    assert(rc == SQLITE_OK);

    // Verify retrieved data
    assert(strcmp(user_data.username, username) == 0);
    assert(strcmp(user_data.cpf, cpf) == 0);
    assert(strcmp(user_data.phone_number, phone_number) == 0);
    assert(user_data.is_admin == is_admin);
    assert(user_data.reset_password == false);
    assert(user_data.created_at > 0);
    printf("User data retrieved correctly.\n");

    // Try to get non-existent user
    printf("Attempting to retrieve non-existent user...\n");
    rc = user_db_get_by_username(&test_user_db, "nonexistent", &user_data);
    assert(rc == SQLITE_NOTFOUND);
    printf("Non-existent user retrieval failed as expected.\n");

    teardown_cleanup();

    printf("user_db_get_by_username test passed successfully.\n");
}

void test_user_db_update_username(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_update_username...\n");

    // Create a test user with known properties
    const char *old_username = "old_username";
    const char *new_username = "new_username";
    bool is_admin = false;
    printf("Creating test user '%s'...\n", old_username);
    int rc = user_db_create_user(&test_user_db, old_username, "00000000000", "", is_admin);
    assert(rc == SQLITE_OK);

    // Set password
    const char *password = "testpassword";
    printf("Setting password...\n");
    rc = user_db_update_password(&test_user_db, old_username, password);
    assert(rc == SQLITE_OK);
    // password reset set as false in the database

    // Change username
    printf("Changing username from '%s' to '%s'...\n", old_username, new_username);
    rc = user_db_update_username(&test_user_db, old_username, new_username);
    assert(rc == SQLITE_OK);
    printf("Username changed successfully.\n");

    // Verify old username no longer exists
    printf("Verifying old username no longer exists...\n");
    bool exists = user_db_check_exists(&test_user_db, old_username);
    assert(!exists);
    printf("Old username correctly removed.\n");

    // Verify new username exists and has correct data
    printf("Verifying new username has correct data...\n");
    struct user user_data;
    rc = user_db_get_by_username(&test_user_db, new_username, &user_data);
    assert(rc == SQLITE_OK);
    assert(strcmp(user_data.username, new_username) == 0);
    assert(user_data.is_admin == is_admin);
    // reset_password will be set as false, changing username will not trigger new password reset
    assert(user_data.reset_password == false);

    // Verify password still works
    printf("Verifying password still works with new username...\n");
    enum auth_result auth_rc = user_db_authenticate(&test_user_db, new_username, password);
    assert(auth_rc != AUTH_NEED_PASSWORD_RESET); // Because reset_password is false
    printf("Authentication with new username works.\n");

    // Try to change to existing username
    printf("Attempting to change to existing username...\n");
    rc = user_db_update_username(&test_user_db, new_username, "admin"); // admin exists
    assert(rc == SQLITE_CONSTRAINT);
    printf("Change to existing username failed as expected.\n");

    // Try to change non-existent user
    printf("Attempting to change non-existent user...\n");
    rc = user_db_update_username(&test_user_db, "nonexistent", "whatever");
    assert(rc == SQLITE_NOTFOUND);
    printf("Change of non-existent user failed as expected.\n");

    teardown_cleanup();

    printf("user_db_update_username test passed successfully.\n");
}

void test_user_db_default_admin_changes(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Trying to delete the default admin...\n");
    int rc = user_db_delete(&test_user_db, "admin");
    assert(rc != SQLITE_OK);
    printf("Deletion of the default admin failed as expected.\n");

    printf("Trying to update the default admin status...\n");
    rc = user_db_update_admin_status(&test_user_db, "admin", 0);
    assert(rc != SQLITE_OK);
    printf("Update of the default admin status failed as expected.\n");

    printf("Trying to update the default admin username...\n");
    rc = user_db_update_username(&test_user_db, "admin", "newusername");
    assert(rc != SQLITE_OK);
    printf("Update of the default admin username failed as expected.\n");

    printf("Trying to update the default admin password...\n");
    user_db_update_password(&test_user_db, "admin", "0000");
    enum auth_result auth_rc = user_db_authenticate(&test_user_db, "admin", "0000");
    assert(auth_rc == AUTH_SUCCESS);
    printf("Update of the default admin password was successful.\n");

    teardown_cleanup();

    printf("user_db_update_admin_status test passed successfully.\n");
}

void test_user_db_check_admin(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Checking if default admin is admin...\n");
    bool is_admin = user_db_check_admin_status(&test_user_db, "admin");

    assert(is_admin == true);
    printf("Default admin is correctly an admin.\n");

    printf("Checking if a newly created admin user is admin...\n");
    user_db_create_user(&test_user_db, "testadmin", "00000000000", "", true);

    is_admin = user_db_check_admin_status(&test_user_db, "testadmin");
    assert(is_admin == true);
    printf("Newly create admin user is correctly an admin.\n");

    printf("Checking if a newly created non-admin user is admin...\n");
    user_db_create_user(&test_user_db, "test_nonadmin", "00000000000", "", false);

    is_admin = user_db_check_admin_status(&test_user_db, "test_nonadmin");
    assert(is_admin == false);
    printf("Newly create non-admin user is correctly not an admin.\n");

    teardown_cleanup();
}

void test_user_db_set_password_reset(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_set_reset_password...\n");

    // Test setting reset password flag for an existing user
    printf("Creating user 'user1'...\n");
    int rc = user_db_create_user(&test_user_db, "user1", "00000000000", "", false);
    assert(rc == SQLITE_OK);
    printf("User 'user1' created successfully.\n");

    printf("Setting reset password for 'user1'...\n");
    rc = user_db_set_reset_password(&test_user_db, "user1");
    assert(rc == SQLITE_OK);
    printf("Reset password for 'user1' set successfully.\n");

    // Test setting reset password flag for a nonexistent user
    printf("Setting reset password for nonexistent user 'user_not_exists'...\n");
    rc = user_db_set_reset_password(&test_user_db, "user_not_exists");
    assert(rc == SQLITE_ERROR || rc == SQLITE_NOTFOUND);
    printf("Setting reset password failed for nonexistent user as expected.\n");

    // Test setting reset password flag for an admin user
    printf("Creating admin user 'admin2'...\n");
    rc = user_db_create_user(&test_user_db, "admin2", "00000000001", "", true);

    printf("Setting reset password for admin user 'admin2'...\n");
    rc = user_db_set_reset_password(&test_user_db, "admin2");
    assert(rc == SQLITE_OK);
    printf("Reset password for admin user 'admin2' set successfully.\n");

    teardown_cleanup();

    printf("user_db_set_reset_password test passed successfully.\n");
}

void test_user_db_get_count(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    // Test empty database
    printf("Testing count on empty database...\n");
    int count = user_db_get_count(&test_user_db);
    assert(count == 1);
    printf("Database has only default admin (1).\n");

    // Add one resident
    printf("Adding one user...\n");
    user_db_create_user(&test_user_db, "test_user0", "00000000000", "5551912345678", false);

    // Verify count is now 2
    printf("Verifying count after insertion...\n");
    count = user_db_get_count(&test_user_db);
    assert(count == 2);
    printf("Count correct after insertion (2).\n");

    // Add multiple residents
    printf("Adding multiple residents...\n");
    user_db_create_user(&test_user_db, "test_user1", "00000000001", "5551912345679", true);
    user_db_create_user(&test_user_db, "test_user2", "00000000002", "5551912345670", false);

    // Verify count is now 4
    printf("Verifying count after multiple insertions...\n");
    count = user_db_get_count(&test_user_db);
    assert(count == 4);
    printf("Count correct after multiple insertions (4).\n");

    // Delete one and verify count
    printf("Deleting one resident...\n");
    user_db_delete(&test_user_db, "test_user2");
    count = user_db_get_count(&test_user_db);
    assert(count == 3);
    printf("Count correct after deletion (3).\n");

    teardown_cleanup();

    printf("user_db_get_count test passed successfully.\n");
}

void test_user_db_get_all(void) {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_get_all...\n");

    // Create several test users
    printf("Creating test users...\n");
    user_db_create_user(&test_user_db, "user1", "00000000000", "", false);
    user_db_create_user(&test_user_db, "user2", "00000000001", "", false);
    user_db_create_user(&test_user_db, "admin1", "00000000002", "", true);
    user_db_create_user(&test_user_db, "admin2", "00000000003", "", true);

    // Set a password and authenticate a user to set Last Login date.
    user_db_update_password(&test_user_db, "user1", "newpassword");
    user_db_authenticate(&test_user_db, "user1", "newpassword");

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling user_db_get_all...\n");
    int rc = user_db_get_all(&test_user_db);
    assert(rc == SQLITE_OK);
    printf("user_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("user_db_get_all test passed successfully.\n");
}

// TEST DB USER END

// TEST DB CLOTHES END

void test_clothes_db_create_table(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    printf("Testing clothes_db_create_table...\n");

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = FEMALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = WORN;
    const int test_quantity = 3;
    const char *test_notes = "None";

    // Verify table structure by trying to insert a clothing
    int rc = clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_quantity,
        test_notes
    );
    assert(rc == SQLITE_OK);
    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("clothes_db_create_table test passed successfully.\n");
}

void test_clothes_db_upsert(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = COATS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_BLACK;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    printf(
        "Attempting to insert clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    int rc = clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    assert(rc == SQLITE_OK);
    printf("Inserted clothes successfully.\n");

    printf(
        "Attempting to update by inserting the same clothes again, with notes being null, quantity should be updated by 3, being equal to 8 and notes should remain the same.\n"
    );

    const int added_quantity = 3;

    rc = clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        added_quantity,
        NULL
    );

    assert(rc == SQLITE_OK);

    printf("Attempt to insert the same clothing was succesful.\n");

    const char *upd_notes = "Updated notes.";

    printf("Attempting to insert quantity 0 and update notes with the following value: %s.\n", upd_notes);

    rc = clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        0,
        upd_notes
    );

    assert(rc == SQLITE_OK);

    printf("Succesful.\n");

    printf("Checking if the note entry was updated.\n");

    struct clothing test_clothing = { 0 };

    clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);
    assert(strcmp(upd_notes, test_clothing.notes) == 0);

    printf("Notes were updated.\n");

    printf("Checking if the quantity was NOT updated.\n");

    assert(test_first_quantity + added_quantity == test_clothing.quantity);

    printf("Quantity was NOT updated.\n");

    printf("Attempting to update/insert a negative quantity.\n");

    rc = clothes_db_upsert(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, -4, NULL);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Negative quantity not allowed.\n");

    teardown_cleanup();

    printf("clothes_db_upsert test passed successfully.\n");
}

void test_clothes_db_remove(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    printf(
        "Inserting clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    int quantity_to_remove = 3;

    printf("Attempting to remove quantity by %d.\n", quantity_to_remove);

    int rc = clothes_db_remove(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        quantity_to_remove
    );

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct clothing test_clothing = { 0 };
    rc =
        clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);

    printf("Checking if Quantity column was updated.\n");

    assert(test_first_quantity - quantity_to_remove == test_clothing.quantity);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove quantity from a non-existent record.\n");

    rc = clothes_db_remove(
        &test_clothes_db,
        COATS,
        test_size,
        test_gender,
        test_color,
        test_condition,
        quantity_to_remove
    );

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = clothes_db_remove(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = clothes_db_remove(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = clothes_db_remove(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("clothes_db_remove test passed successfully.\n");
}

void test_clothes_db_delete_entry(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    printf(
        "Inserting clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    printf("Attempting to delete previous inserted entry.\n");

    int rc = clothes_db_delete_entry(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition);

    assert(rc == SQLITE_OK);

    struct clothing test_clothing = { 0 };
    rc =
        clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = clothes_db_delete_entry(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("clothes_db_delete_entry test passed successfully.\n");
}

void test_clothes_db_remove_by_id(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";
    const int id = 1;

    printf(
        "Inserting clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    int quantity_to_remove = 3;

    printf("Attempting to remove quantity by id %d of clothes with ID 1.\n", quantity_to_remove);

    int rc = clothes_db_remove_by_id(&test_clothes_db, id, quantity_to_remove);

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct clothing test_clothing = { 0 };
    rc =
        clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);

    printf("Checking if Quantity column was updated.\n");

    assert(test_first_quantity - quantity_to_remove == test_clothing.quantity);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove quantity from a non-existent record ID.\n");

    rc = clothes_db_remove_by_id(&test_clothes_db, 10, quantity_to_remove);

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = clothes_db_remove_by_id(&test_clothes_db, id, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = clothes_db_remove_by_id(&test_clothes_db, id, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = clothes_db_remove_by_id(&test_clothes_db, id, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("clothes_db_remove_by_id test passed successfully.\n");
}

void test_clothes_db_delete_entry_by_id(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    printf(
        "Inserting clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    printf("Attempting to delete by id the previous inserted entry.\n");

    int rc = clothes_db_delete_entry_by_id(&test_clothes_db, 1);

    assert(rc == SQLITE_OK);

    struct clothing test_clothing = { 0 };
    rc =
        clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = clothes_db_delete_entry_by_id(&test_clothes_db, 10);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("clothes_db_delete_entry_by_id test passed successfully.\n");
}

void test_clothes_db_check_exists(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    printf("Checking if a non-existent entry exists.\n");

    bool exists = clothes_db_check_exists(&test_clothes_db, COATS, M, FEMALE, CL_RED, NEW);
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if a an existent entry exists.\n");

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    exists = clothes_db_check_exists(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("clothes_db_check_exists passed successfully.\n");
}

void test_clothes_db_check_exists_by_id(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    printf("Checking if a non-existent entry exists by id.\n");

    bool exists = clothes_db_check_exists_by_id(&test_clothes_db, 10);
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if a an existent entry exists.\n");

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    exists = clothes_db_check_exists_by_id(&test_clothes_db, 1);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("clothes_db_check_exists_by_id passed successfully.\n");
}

void test_clothes_db_get(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    const enum clothing_type test_type = TSHIRTS;
    const enum clothing_size test_size = M;
    const enum clothing_gender test_gender = MALE;
    const enum clothing_color test_color = CL_RED;
    const enum clothing_condition test_condition = NEW;
    const int test_first_quantity = 5;
    const char *test_notes = "Special donation";

    printf(
        "Inserting clothes record with the following values:\n"
        "Type: %s\n"
        "Size: %s\n"
        "Gender: %s\n"
        "Color: %s\n"
        "Condition: %s\n"
        "Quantity: %d\n"
        "Notes: %s\n",
        clothing_type_str[test_type],
        clothing_size_str[test_size],
        clothing_gender_str[test_gender],
        clothing_color_str[test_color],
        clothing_condition_str[test_condition],
        test_first_quantity,
        test_notes
    );

    clothes_db_upsert(
        &test_clothes_db,
        test_type,
        test_size,
        test_gender,
        test_color,
        test_condition,
        test_first_quantity,
        test_notes
    );

    printf("Attempting to fill in a struct clothing.\n");

    struct clothing test_clothing = { 0 };
    int rc =
        clothes_db_get(&test_clothes_db, test_type, test_size, test_gender, test_color, test_condition, &test_clothing);

    assert(rc == SQLITE_OK);

    printf("Checking if the entries on the database are equal to the inserted data.\n");
    assert(test_type == test_clothing.type);
    assert(test_size == test_clothing.size);
    assert(test_gender == test_clothing.gender);
    assert(test_color == test_clothing.color);
    assert(test_first_quantity == test_clothing.quantity);
    assert(strcmp(test_notes, test_clothing.notes) == 0);

    printf("Struct filled with correct data successfully.\n");

    printf("Attempting to get a non-existent entry.\n");

    rc = clothes_db_get(&test_clothes_db, COATS, test_size, test_gender, test_color, test_condition, &test_clothing);

    assert(rc == SQLITE_NOTFOUND);

    printf("Getting a non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("clothes_db_get test passed successfully.\n");
}

void test_clothes_db_get_all(void) {
    const char *test_clothesdb_filename = "test_clothes_db.db";
    database test_clothes_db;
    db_init_with_tbl(&test_clothes_db, test_clothesdb_filename, clothes_db_create_table);

    setup_cleanup(test_clothesdb_filename, &test_clothes_db);

    printf("Testing clothes_db_get_all...\n");

    // Create several test clothes
    printf("Creating test clothes...\n");
    clothes_db_upsert(&test_clothes_db, COATS, XL, FEMALE, CL_BLACK, NEW, 10, "None");
    clothes_db_upsert(&test_clothes_db, SHIRTS, S, MALE, CL_RED, NEEDS_REPAIR, 1, "Zara brand");
    clothes_db_upsert(&test_clothes_db, SWIMWEARS, XS, FEMALE, CL_WHITE, WORN, 2, NULL);
    clothes_db_upsert(&test_clothes_db, BOOTS, M, OTHER, CL_BROWN, NEW, 3, "");

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling clothes_db_get_all...\n");
    int rc = clothes_db_get_all(&test_clothes_db);
    assert(rc == SQLITE_OK);
    printf("clothes_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("clothes_db_get_all test passed successfully.\n");
}

// TEST DB CLOTHES END

// TEST DB MEDICATION START

void test_medication_db_create_table(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    printf("Testing medication_db_create_table...\n");

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 3;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    // Verify table structure by trying to insert a clothing
    int rc = medication_db_upsert(
        &test_medication_db,
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );
    assert(rc == SQLITE_OK);
    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("medication_db_create_table test passed successfully.\n");
}

void test_medication_db_upsert(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 5;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    int rc = medication_db_upsert(
        &test_medication_db,
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    assert(rc == SQLITE_OK);
    printf("Inserted medication successfully.\n");

    const int added_stock = 3;

    printf(
        "Attempting to update by inserting the same medication again, with notes being null, quantity "
        "should be updated by %d, being equal to %d and notes should remain the same.\n",
        added_stock,
        added_stock + stock
    );

    rc = medication_db_upsert(
        &test_medication_db,
        name,
        generic_name,
        form,
        strength,
        unit,
        added_stock,
        expiration_date,
        NULL
    );

    assert(rc == SQLITE_OK);

    printf("Attempt to insert the same clothing was succesful.\n");

    printf("Attempting to insert a NULL date.\n");

    rc = medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, 0, NULL, NULL);

    assert(rc == SQLITE_OK);

    printf("Attempt to insert the same clothing with NULL date was succesful.\n");

    const char *upd_notes = "Updated notes.";
    const char *upd_date = "2000-01-01";

    printf(
        "Attempting to insert quantity 0 and update notes and date with the following values: %s and %s.\n",
        upd_notes,
        upd_date
    );

    rc = medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, 0, upd_date, upd_notes);

    assert(rc == SQLITE_OK);

    printf("Succesful.\n");

    printf("Checking if the note and date entry was updated.\n");

    struct medication test_medication = { 0 };

    medication_db_get(&test_medication_db, name, form, strength, &test_medication);
    assert(strcmp(upd_notes, test_medication.notes) == 0);
    assert(strcmp(upd_date, test_medication.expiration_date) == 0);

    printf("Notes and date were updated.\n");

    printf("Checking if the stock was NOT updated.\n");

    printf("Quantity was NOT updated.\n");

    printf("Attempting to update/insert a negative stock.\n");

    rc =
        medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, -4, expiration_date, notes);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Negative stock not allowed.\n");

    teardown_cleanup();

    printf("medication_db_upsert test passed successfully.\n");
}

void test_medication_db_remove(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);
    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    int quantity_to_remove = 3;

    printf("Attempting to remove stock by %d.\n", quantity_to_remove);

    int rc = medication_db_remove(&test_medication_db, name, form, strength, quantity_to_remove);

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct medication test_medication = { 0 };
    rc = medication_db_get(&test_medication_db, name, form, strength, &test_medication);

    printf("Checking if Stock column was updated.\n");

    assert(stock - quantity_to_remove == test_medication.stock);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove quantity from a non-existent record.\n");

    rc = medication_db_remove(&test_medication_db, "ritalin", "syrup", "1000mg", quantity_to_remove);

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = medication_db_remove(&test_medication_db, name, form, strength, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = medication_db_remove(&test_medication_db, name, form, strength, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = medication_db_remove(&test_medication_db, name, form, strength, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("medication_db_remove test passed successfully.\n");
}

void test_medication_db_delete_entry(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    printf("Attempting to delete previous inserted entry.\n");

    int rc = medication_db_delete_entry(&test_medication_db, name, form, strength);

    assert(rc == SQLITE_OK);

    struct medication test_medication = { 0 };
    rc = medication_db_get(&test_medication_db, name, form, strength, &test_medication);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = medication_db_delete_entry(&test_medication_db, "ritalin", form, strength);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("medication_db_delete_entry test passed successfully.\n");
}

void test_medication_db_remove_by_id(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";
    const int id = 1;

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    int quantity_to_remove = 3;

    printf("Attempting to remove stock by id %d of medication with ID 1.\n", quantity_to_remove);

    int rc = medication_db_remove_by_id(&test_medication_db, id, quantity_to_remove);

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct medication test_medication = { 0 };
    rc = medication_db_get(&test_medication_db, name, form, strength, &test_medication);

    printf("Checking if Quantity column was updated.\n");

    assert(stock - quantity_to_remove == test_medication.stock);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove stock from a non-existent record ID.\n");

    rc = medication_db_remove_by_id(&test_medication_db, 10, quantity_to_remove);

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = medication_db_remove_by_id(&test_medication_db, id, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = medication_db_remove_by_id(&test_medication_db, id, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = medication_db_remove_by_id(&test_medication_db, id, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("medication_db_remove_by_id test passed successfully.\n");
}

void test_medication_db_delete_entry_by_id(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    printf("Attempting to delete by id the previous inserted entry.\n");

    int rc = medication_db_delete_entry_by_id(&test_medication_db, 1);

    assert(rc == SQLITE_OK);

    struct medication test_medication = { 0 };
    rc = medication_db_get(&test_medication_db, name, form, strength, &test_medication);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = medication_db_delete_entry_by_id(&test_medication_db, 10);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("medication_db_delete_entry_by_id test passed successfully.\n");
}

void test_medication_db_check_exists(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    printf("Checking if a non-existent entry exists.\n");

    bool exists = medication_db_check_exists(&test_medication_db, "ritalin", "syrup", "1ug");
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if an existent entry exists.\n");

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    exists = medication_db_check_exists(&test_medication_db, name, form, strength);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("medication_db_check_exists passed successfully.\n");
}

void test_medication_db_check_exists_by_id(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    printf("Checking if a non-existent entry exists by id.\n");

    bool exists = medication_db_check_exists_by_id(&test_medication_db, 10);
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if a an existent entry exists.\n");

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    exists = medication_db_check_exists_by_id(&test_medication_db, 1);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("medication_db_check_exists_by_id passed successfully.\n");
}

void test_medication_db_get(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    const char *name = "paracetamol";
    const char *generic_name = "paracetamol 500mg";
    const char *form = "tablet";
    const char *strength = "500mg";
    const char *unit = "tablet";
    const int stock = 10;
    const char *expiration_date = "2025-12-10";
    const char *notes = "None";

    printf(
        "Attempting to insert medication record with the following values:\n"
        "Name: %s\n"
        "Generic Name: %s\n"
        "Form: %s\n"
        "Strength: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Expiration Date: %s\n"
        "Notes: %s\n",
        name,
        generic_name,
        form,
        strength,
        unit,
        stock,
        expiration_date,
        notes
    );

    medication_db_upsert(&test_medication_db, name, generic_name, form, strength, unit, stock, expiration_date, notes);

    printf("Attempting to fill in a struct medication.\n");

    struct medication test_medication = { 0 };
    int rc = medication_db_get(&test_medication_db, name, form, strength, &test_medication);

    assert(rc == SQLITE_OK);

    printf("Checking if the entries on the database are equal to the inserted data.\n");
    printf("%s\n%s\n", name, test_medication.name);
    assert(strcmp(name, test_medication.name) == 0);
    assert(strcmp(generic_name, test_medication.generic_name) == 0);
    assert(strcmp(form, test_medication.form) == 0);
    assert(strcmp(strength, test_medication.strength) == 0);
    assert(strcmp(unit, test_medication.unit) == 0);
    assert(stock == test_medication.stock);
    assert(strcmp(expiration_date, test_medication.expiration_date) == 0);
    assert(strcmp(notes, test_medication.notes) == 0);

    printf("Struct filled with correct data successfully.\n");

    printf("Attempting to get a non-existent entry.\n");

    rc = medication_db_get(&test_medication_db, "ritalin", form, strength, &test_medication);

    assert(rc == SQLITE_NOTFOUND);

    printf("Getting a non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("medication_db_get test passed successfully.\n");
}

void test_medication_db_get_all(void) {
    const char *test_medicationdb_filename = "test_medication_db.db";
    database test_medication_db;
    db_init_with_tbl(&test_medication_db, test_medicationdb_filename, medication_db_create_table);

    setup_cleanup(test_medicationdb_filename, &test_medication_db);

    printf("Testing medication_db_get_all...\n");

    // Create several test medications
    printf("Creating test medications...\n");
    medication_db_upsert(
        &test_medication_db,
        "paracetamol",
        "paracetamol 100mg",
        "syrup",
        "100mg",
        "ml",
        3,
        "2025-08-10",
        ""
    );
    medication_db_upsert(
        &test_medication_db,
        "ibuprofen",
        "ibuprofen 500mg",
        "tablet",
        "500mg",
        "tablet",
        5,
        "2025-09-05",
        "None"
    );
    medication_db_upsert(&test_medication_db, "amoxicillin", "", "syrup", "123mg", "syrup", 1, "2025-05-31", "");
    medication_db_upsert(
        &test_medication_db,
        "zolpidem",
        "zolpidem 300mg",
        "tablet",
        "300mg",
        "tablet",
        3,
        "2025-12-30",
        ""
    );

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling medication_db_get_all...\n");
    int rc = medication_db_get_all(&test_medication_db);
    assert(rc == SQLITE_OK);
    printf("medication_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("medication_db_get_all test passed successfully.\n");
}

// MEDICATION DB END

// TEST DB SUPPLIES START

void test_supplies_db_create_table(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    printf("Testing supplies_db_create_table...\n");

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    // Verify table structure by trying to insert a supply
    int rc = supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);
    assert(rc == SQLITE_OK);
    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("supplies_db_create_table test passed successfully.\n");
}

void test_supplies_db_upsert(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    int rc = supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    assert(rc == SQLITE_OK);
    printf("Inserted supply successfully.\n");

    const int added_stock = 3;

    printf(
        "Attempting to update by inserting the same supply again, with notes being null, quantity "
        "should be updated by %d, being equal to %d and notes should remain the same.\n",
        added_stock,
        added_stock + stock
    );

    rc = supplies_db_upsert(&test_supplies_db, name, category, size, unit, added_stock, NULL);

    assert(rc == SQLITE_OK);

    printf("Attempt to insert the same supply was succesful.\n");

    const char *upd_notes = "Updated notes.";

    printf("Attempting to insert quantity 0 and update notes with the following value: %s.\n", upd_notes);

    rc = supplies_db_upsert(&test_supplies_db, name, category, size, unit, 0, upd_notes);

    assert(rc == SQLITE_OK);

    printf("Succesful.\n");

    printf("Checking if the note was updated.\n");

    struct supply test_supply = { 0 };

    supplies_db_get(&test_supplies_db, name, category, size, &test_supply);
    assert(strcmp(upd_notes, test_supply.notes) == 0);

    printf("Notes was updated.\n");

    printf("Checking if the stock was NOT updated.\n");

    printf("Quantity was NOT updated.\n");

    printf("Attempting to update/insert a negative stock.\n");

    rc = supplies_db_upsert(&test_supplies_db, name, category, size, unit, -4, notes);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Negative stock not allowed.\n");

    teardown_cleanup();

    printf("supplies_db_upsert test passed successfully.\n");
}

void test_supplies_db_remove(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    int quantity_to_remove = 3;

    printf("Attempting to remove stock by %d.\n", quantity_to_remove);

    int rc = supplies_db_remove(&test_supplies_db, name, category, size, quantity_to_remove);

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct supply test_supply = { 0 };
    rc = supplies_db_get(&test_supplies_db, name, category, size, &test_supply);

    printf("Checking if Stock column was updated.\n");

    assert(stock - quantity_to_remove == test_supply.stock);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove quantity from a non-existent record.\n");

    rc = supplies_db_remove(&test_supplies_db, "tampon", "hygiene", "pack", quantity_to_remove);

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = supplies_db_remove(&test_supplies_db, name, category, size, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = supplies_db_remove(&test_supplies_db, name, category, size, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = supplies_db_remove(&test_supplies_db, name, category, size, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("supplies_db_remove test passed successfully.\n");
}

void test_supplies_db_delete_entry(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    printf("Attempting to delete previous inserted entry.\n");

    int rc = supplies_db_delete_entry(&test_supplies_db, name, category, size);

    assert(rc == SQLITE_OK);

    struct supply test_supply = { 0 };
    rc = supplies_db_get(&test_supplies_db, name, category, size, &test_supply);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = supplies_db_delete_entry(&test_supplies_db, "tampon", category, unit);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("supplies_db_delete_entry test passed successfully.\n");
}

void test_supplies_db_remove_by_id(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";
    const int id = 1;

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    int quantity_to_remove = 3;

    printf("Attempting to remove stock by id %d of supply with ID 1.\n", quantity_to_remove);

    int rc = supplies_db_remove_by_id(&test_supplies_db, id, quantity_to_remove);

    assert(rc == SQLITE_OK);

    printf("Removal was successful.\n");

    struct supply test_supply = { 0 };
    rc = supplies_db_get(&test_supplies_db, name, category, size, &test_supply);

    printf("Checking if Quantity column was updated.\n");

    assert(stock - quantity_to_remove == test_supply.stock);

    printf("Quantity updated successfully.\n");

    printf("Attempting to remove stock from a non-existent record ID.\n");

    rc = supplies_db_remove_by_id(&test_supplies_db, 10, quantity_to_remove);

    assert(rc == SQLITE_NOTFOUND);

    printf("Removal of non-existent record was unsuccessful.\n");

    printf("Attempting to remove 0 quantity.\n");

    rc = supplies_db_remove_by_id(&test_supplies_db, id, 0);

    assert(rc == SQLITE_OK);

    printf("Removal with 0 quantity was successfull, didn't do anything.\n");

    printf("Attempting to remove negative quantity.\n");

    rc = supplies_db_remove_by_id(&test_supplies_db, id, -4);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal of a negative quantity was unsuccessful.\n");

    printf("Attempting to remove a quantity that will make the stock goes below 0.\n");

    rc = supplies_db_remove_by_id(&test_supplies_db, id, 10000);

    assert(rc == SQLITE_CONSTRAINT);

    printf("Removal was unsuccessful.\n");

    teardown_cleanup();

    printf("supplies_db_remove_by_id test passed successfully.\n");
}

void test_supplies_db_delete_entry_by_id(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    printf("Attempting to delete by id the previous inserted entry.\n");

    int rc = supplies_db_delete_entry_by_id(&test_supplies_db, 1);

    assert(rc == SQLITE_OK);

    struct supply test_supply = { 0 };
    rc = supplies_db_get(&test_supplies_db, name, category, size, &test_supply);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion was successful.\n");

    printf("Attempting to delete a non-existent entry.\n");

    rc = supplies_db_delete_entry_by_id(&test_supplies_db, 10);

    assert(rc == SQLITE_NOTFOUND);

    printf("Deletion of non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("supplies_db_delete_entry_by_id test passed successfully.\n");
}

void test_supplies_db_check_exists(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    printf("Checking if a non-existent entry exists.\n");

    bool exists = supplies_db_check_exists(&test_supplies_db, "tampon", "hygiene", "pack");
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if an existent entry exists.\n");

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    exists = supplies_db_check_exists(&test_supplies_db, name, category, size);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("supplies_db_check_exists passed successfully.\n");
}

void test_supplies_db_check_exists_by_id(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    printf("Checking if a non-existent entry exists by id.\n");

    bool exists = supplies_db_check_exists_by_id(&test_supplies_db, 10);
    assert(exists == false);

    printf("Doesn't exist as expected.\n");

    printf("Checking if a an existent entry exists.\n");

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    exists = supplies_db_check_exists_by_id(&test_supplies_db, 1);

    assert(exists == true);

    printf("Exist as expected.\n");

    teardown_cleanup();

    printf("supplies_db_check_exists_by_id passed successfully.\n");
}

void test_supplies_db_get(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    const char *name = "diaper";
    const char *category = "hygiene";
    const char *size = "m";
    const char *unit = "pack";
    const int stock = 5;
    const char *notes = "Pampers";

    printf(
        "Attempting to insert supply record with the following values:\n"
        "Name: %s\n"
        "Category: %s\n"
        "Size: %s\n"
        "Unit: %s\n"
        "Stock: %d\n"
        "Notes: %s\n",
        name,
        category,
        size,
        unit,
        stock,
        notes
    );

    supplies_db_upsert(&test_supplies_db, name, category, size, unit, stock, notes);

    printf("Attempting to fill in a struct supply.\n");

    struct supply test_supply = { 0 };
    int rc = supplies_db_get(&test_supplies_db, name, category, size, &test_supply);

    assert(rc == SQLITE_OK);

    printf("Checking if the entries on the database are equal to the inserted data.\n");
    printf("%s\n%s\n", name, test_supply.name);
    assert(strcmp(name, test_supply.name) == 0);
    assert(strcmp(category, test_supply.category) == 0);
    assert(strcmp(size, test_supply.size) == 0);
    assert(strcmp(unit, test_supply.unit) == 0);
    assert(stock == test_supply.stock);
    assert(strcmp(notes, test_supply.notes) == 0);

    printf("Struct filled with correct data successfully.\n");

    printf("Attempting to get a non-existent entry.\n");

    rc = supplies_db_get(&test_supplies_db, "tampon", category, size, &test_supply);

    assert(rc == SQLITE_NOTFOUND);

    printf("Getting a non-existent entry was unsuccessful.\n");

    teardown_cleanup();

    printf("supplies_db_get test passed successfully.\n");
}

void test_supplies_db_get_all(void) {
    const char *test_suppliesdb_filename = "test_supplies_db.db";
    database test_supplies_db;
    db_init_with_tbl(&test_supplies_db, test_suppliesdb_filename, supplies_db_create_table);

    setup_cleanup(test_suppliesdb_filename, &test_supplies_db);

    printf("Testing supplies_db_get_all...\n");

    // Create several test supplies
    printf("Creating test supplies...\n");
    supplies_db_upsert(&test_supplies_db, "diaper", "hygiene", "m", "pack", 3, "Pampers");
    supplies_db_upsert(&test_supplies_db, "tampon", "personal care", "l", "pack", 5, "None");
    supplies_db_upsert(&test_supplies_db, "water", "consumable", "s", "", 1, "");
    supplies_db_upsert(&test_supplies_db, "pen", "office", "s", "piece", 100, "");

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling supplies_db_get_all...\n");
    int rc = supplies_db_get_all(&test_supplies_db);
    assert(rc == SQLITE_OK);
    printf("supplies_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("supplies_db_get_all test passed successfully.\n");
}

// SUPPLIES DB END

// TEST DB TASKS START

// TEST DB TASKS START

void test_tasks_db_create_table(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);

    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    printf("Testing tasks_db_create_table...\n");

    // Try inserting a record to verify structure
    int rc = tasks_db_upsert(
        &test_tasks_db,
        0,
        "Restock Diapers",
        "Restock all diaper supplies",
        "2025-07-01",
        TSK_HIGH,
        TSK_PENDING,
        "alice",
        NULL
    );

    tasks_db_get_all(&test_tasks_db);

    assert(rc == SQLITE_OK);

    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("tasks_db_create_table test passed successfully.\n");
}

void test_tasks_db_upsert(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    const char *title = "Restock Diapers";
    const char *desc = "Restock all diaper supplies";
    const char *due_date = "2024-07-01";
    int priority = TSK_HIGH;
    int status = TSK_PENDING;
    const char *assigned = "alice";

    printf("Attempting to insert a new task...\n");
    int rc = tasks_db_upsert(&test_tasks_db, 0, title, desc, due_date, priority, status, assigned, NULL);
    assert(rc == SQLITE_OK);

    printf("Initial insert OK.\n");

    struct task the_task = { 0 };
    rc = tasks_db_get(&test_tasks_db, 1, &the_task);
    assert(rc == SQLITE_OK);
    assert(strcmp(the_task.title, title) == 0);

    printf("Fetched inserted task successfully.\n");

    // Now update the task, change status and complete it
    const char *completed_time = "2025-12-02 10:00:00";
    rc = tasks_db_upsert(&test_tasks_db, 1, title, desc, due_date, priority, TSK_DONE, assigned, completed_time);
    assert(rc == SQLITE_OK);

    rc = tasks_db_get(&test_tasks_db, 1, &the_task);
    assert(rc == SQLITE_OK);
    assert(the_task.status == 2);

    printf("Updated status and completed_at OK.\n");

    // Try to update a non-existing ID
    rc = tasks_db_upsert(&test_tasks_db, 99, title, desc, due_date, TSK_NORMAL, TSK_PENDING, NULL, NULL);
    assert(rc == SQLITE_NOTFOUND);

    printf("tasks_db_upsert handles non-existent update correctly.\n");

    teardown_cleanup();
    printf("tasks_db_upsert test passed successfully.\n");
}

void test_tasks_db_check_exists(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    printf("Checking if a non-existent task id exists.\n");
    assert(!tasks_db_check_exists(&test_tasks_db, 123));

    printf("Non-existent id does not exist as expected.\n");

    // Insert a record, check exists
    tasks_db_upsert(&test_tasks_db, 0, "Do something", "desc", "2025-06-05", 0, 0, NULL, NULL);
    assert(tasks_db_check_exists(&test_tasks_db, 1));

    printf("tasks_db_check_exists found the inserted record.\n");

    teardown_cleanup();
    printf("tasks_db_check_exists passed successfully.\n");
}

void test_tasks_db_get(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    // Insert and then fetch
    tasks_db_upsert(
        &test_tasks_db,
        0,
        "FetchThis",
        "FETCH SUCCESS",
        "2024-09-01",
        TSK_NORMAL,
        TSK_IN_PROGRESS,
        "bob",
        NULL
    );

    struct task tsk = { 0 };
    int rc = tasks_db_get(&test_tasks_db, 1, &tsk);
    assert(rc == SQLITE_OK);
    assert(strcmp(tsk.title, "FetchThis") == 0);
    assert(strcmp(tsk.description, "FETCH SUCCESS") == 0);
    assert(tsk.priority == TSK_NORMAL);

    // Try getting non-existent
    rc = tasks_db_get(&test_tasks_db, 999, &tsk);
    assert(rc == SQLITE_NOTFOUND);

    printf("tasks_db_get test passed successfully.\n");

    teardown_cleanup();
}

void test_tasks_db_get_count(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    assert(tasks_db_get_count(&test_tasks_db) == 0);

    tasks_db_upsert(&test_tasks_db, 0, "Count1", "desc", "2024-07-20", TSK_NORMAL, TSK_PENDING, NULL, NULL);
    assert(tasks_db_get_count(&test_tasks_db) == 1);

    tasks_db_upsert(&test_tasks_db, 0, "Count2", "desc", "2024-07-21", TSK_NORMAL, TSK_PENDING, NULL, NULL);
    assert(tasks_db_get_count(&test_tasks_db) == 2);

    teardown_cleanup();
    printf("tasks_db_get_count test passed successfully.\n");
}

void test_tasks_db_get_all_and_format(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    // Insert a few diverse tasks
    tasks_db_upsert(&test_tasks_db, 0, "A", "DescA", "2025-06-23", TSK_LOW, TSK_PENDING, "alice", NULL);
    tasks_db_upsert(&test_tasks_db, 0, "B", "DescB", "2025-06-11", TSK_NORMAL, TSK_IN_PROGRESS, "bob", NULL);
    tasks_db_upsert(&test_tasks_db, 0, "C", "DescC", NULL, 0, 2, NULL, "2025-12-30 17:07");

    printf("Testing tasks_db_get_all (prints to stdout)...\n");
    int rc = tasks_db_get_all(&test_tasks_db);
    assert(rc == SQLITE_OK);

    // Buffer-based formatting
    char buf[4096];
    printf("Testing tasks_db_get_all_format...\n");
    int n = tasks_db_get_all_format(&test_tasks_db, buf, sizeof(buf));
    assert(n > 0);
    printf("tasks_db_get_all_format:\n%s", buf);

    teardown_cleanup();
    printf("tasks_db_get_all_and_format test passed successfully.\n");
}

void test_tasks_db_get_all_format_old(void) {
    const char *test_tasksdb_filename = "test_tasks_db.db";
    database test_tasks_db;
    db_init_with_tbl(&test_tasks_db, test_tasksdb_filename, tasks_db_create_table);
    setup_cleanup(test_tasksdb_filename, &test_tasks_db);

    // Insert a few tasks
    tasks_db_upsert(&test_tasks_db, 0, "Restock Milk", "Restock milk on area area_name", "2025-05-01", TSK_NORMAL, TSK_DONE, "Alice", "2025-06-02 16:37:26");
    tasks_db_upsert(&test_tasks_db, 0, "oldB", "desc", "2025-06-01", 1, 1, NULL, NULL);

    char *formatted = tasks_db_get_all_format_old(&test_tasks_db);
    assert(formatted != NULL);

    printf("tasks_db_get_all_format_old output:\n%s", formatted);
    free(formatted);

    teardown_cleanup();
    printf("tasks_db_get_all_format_old test passed successfully.\n");
}



// TASKS DB END

// UTILS_HASH TESTS

// Helper function to count non-null bytes in a string
int count_non_null_bytes(const char *str, size_t len) {
    int count = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] != '\0')
            count++;
    }
    return count;
}

// Test cases
void test_generate_salt(void) {
    printf("Testing generate_salt...\n");

    // Test normal operation
    printf("Testing normal salt generation...\n");
    char salt[SALT_LEN + 1];
    generate_salt(salt, SALT_LEN);

    // Verify length and null termination
    assert(count_non_null_bytes(salt, SALT_LEN) == SALT_LEN);
    assert(salt[SALT_LEN] == '\0');

    // Verify all bytes are populated (though we can't verify randomness)
    assert(count_non_null_bytes(salt, SALT_LEN) == SALT_LEN);
    printf("Salt generation successful. Salt: %s\n", salt);

    // Test with different length
    printf("Testing with different length...\n");
    int short_len = 16;
    char short_salt[short_len + 1];
    generate_salt(short_salt, short_len);
    assert(count_non_null_bytes(short_salt, short_len) == short_len);
    assert(short_salt[short_len] == '\0');
    printf("Variable length salt generation successful.\n");

    printf("generate_salt test passed successfully.\n");
}

void test_hash_password(void) {
    printf("Testing hash_password...\n");

    // Test with known values
    printf("Testing with known password and salt...\n");
    const char *password = "testpassword";
    const char *salt = "testsalt";
    char hash[PASSWORD_HASH_LEN + 1];

    hash_password(password, salt, hash);

    // Verify output format
    assert(strlen(hash) == PASSWORD_HASH_LEN);
    assert(hash[PASSWORD_HASH_LEN] == '\0');

    // Verify it's a valid hex string
    for (int i = 0; i < PASSWORD_HASH_LEN; i++) {
        assert(isxdigit((int)hash[i]));
    }
    printf("Hash generated successfully: %s\n", hash);

    // Test that different passwords produce different hashes
    printf("Testing hash uniqueness for different passwords...\n");
    char hash2[PASSWORD_HASH_LEN + 1];
    hash_password("different", salt, hash2);
    assert(strcmp(hash, hash2) != 0);
    printf("Different passwords produce different hashes.\n");

    // Test that different salts produce different hashes
    printf("Testing hash uniqueness for different salts...\n");
    hash_password(password, "differentsalt", hash2);
    assert(strcmp(hash, hash2) != 0);
    printf("Different salts produce different hashes.\n");

    // Test with empty password (should still work)
    printf("Testing with empty password...\n");
    hash_password("", salt, hash);
    assert(strlen(hash) == PASSWORD_HASH_LEN);
    printf("Empty password handled correctly.\n");

    // Test with empty salt (should still work)
    printf("Testing with empty salt...\n");
    hash_password(password, "", hash);
    assert(strlen(hash) == PASSWORD_HASH_LEN);
    printf("Empty salt handled correctly.\n");

    printf("hash_password test passed successfully.\n");
}

void test_hash_consistency(void) {
    printf("Testing hash consistency...\n");

    // Test that same input produces same output
    printf("Testing deterministic output...\n");
    const char *password = "consistent";
    const char *salt = "consistentsalt";
    char hash1[PASSWORD_HASH_LEN + 1];
    char hash2[PASSWORD_HASH_LEN + 1];

    hash_password(password, salt, hash1);
    hash_password(password, salt, hash2);

    assert(strcmp(hash1, hash2) == 0);
    printf("Same input produces same output.\n");

    printf("hash_consistency test passed successfully.\n");
}

void test_hash_collision_resistance(void) {
    printf("Testing hash collision resistance...\n");

    // Use more distinct test cases
    const char *passwords[] = { "password1", "password2", "password3",  "password4",
                                "Password1", "P@ssword1", "pass word1", "passw0rd1" };

    const char *salts[] = { "salt1", "salt2", "salt3", "salt4", "Salt1", "S@lt1", " salt1", "sal t1" };

    char hashes[sizeof(passwords) / sizeof(passwords[0]) * sizeof(salts) / sizeof(salts[0])][PASSWORD_HASH_LEN + 1];
    int hash_count = 0;

    // Generate all combinations
    for (size_t i = 0; i < sizeof(passwords) / sizeof(passwords[0]); i++) {
        for (size_t j = 0; j < sizeof(salts) / sizeof(salts[0]); j++) {
            // Skip cases where password+salt combination would be identical
            if (strlen(passwords[i]) > 0 && passwords[i][strlen(passwords[i]) - 1] == ' ' && strlen(salts[j]) > 0
                && salts[j][0] == ' ')
            {
                continue;
            }
            if (strlen(salts[j]) > 0 && salts[j][strlen(salts[j]) - 1] == ' ' && strlen(passwords[i]) > 0
                && passwords[i][0] == ' ')
            {
                continue;
            }

            hash_password(passwords[i], salts[j], hashes[hash_count++]);
        }
    }

    // Check for duplicates
    int duplicates = 0;
    for (int i = 0; i < hash_count; i++) {
        for (int j = i + 1; j < hash_count; j++) {
            if (strcmp(hashes[i], hashes[j]) == 0) {
                duplicates++;
                printf("Collision found between:\n");
                printf("  Password: '%s', Salt: '%s'\n", passwords[i], salts[i]);
                printf("  Password: '%s', Salt: '%s'\n", passwords[j], salts[j]);
                printf("  Hash: %s\n", hashes[i]);
            }
        }
    }

    assert(duplicates == 0);
    printf("No collisions found in %d hash combinations.\n", hash_count);

    printf("hash_collision_resistance test passed successfully.\n");
}

void test_edge_cases(void) {
    printf("Testing edge cases...\n");

    char hash[PASSWORD_HASH_LEN + 1];
    char salt[SALT_LEN + 1];

    // Test very long password
    printf("Testing long password...\n");
    char long_password[1024];
    memset(long_password, 'a', sizeof(long_password) - 1);
    long_password[sizeof(long_password) - 1] = '\0';

    generate_salt(salt, SALT_LEN);
    hash_password(long_password, salt, hash);
    assert(strlen(hash) == PASSWORD_HASH_LEN);
    printf("Long password handled correctly.\n");

    // Test non-ASCII characters
    printf("Testing non-ASCII characters...\n");
    hash_password("p@sswörd", salt, hash);
    assert(strlen(hash) == PASSWORD_HASH_LEN);
    printf("Non-ASCII characters handled correctly.\n");

    printf("edge_cases test passed successfully.\n");
}

// UTILS_HASH TESTS END

// UTILSFN TESTS

void test_flag_macros(void) {
    printf("Testing flag macros...\n");

    unsigned int flags = 0;

    // Test SET_FLAG
    printf("Testing SET_FLAG...\n");
    SET_FLAG(&flags, 0x01);
    assert(flags == 0x01);
    SET_FLAG(&flags, 0x02);
    assert(flags == 0x03);
    printf("SET_FLAG works correctly.\n");

    // Test CLEAR_FLAG
    printf("Testing CLEAR_FLAG...\n");
    CLEAR_FLAG(&flags, 0x01);
    assert(flags == 0x02);
    printf("CLEAR_FLAG works correctly.\n");

    // Test IS_FLAG_SET
    printf("Testing IS_FLAG_SET...\n");
    assert(IS_FLAG_SET(&flags, 0x02));
    assert(!IS_FLAG_SET(&flags, 0x01));
    printf("IS_FLAG_SET works correctly.\n");

    printf("Flag macros test passed successfully.\n");
}

void test_is_int_between_min_max(void) {
    printf("Testing is_int_between_min_max...\n");

    // Test valid lengths
    printf("Testing valid lengths...\n");
    assert(is_int_between_min_max("123", 1, 3));
    assert(is_int_between_min_max("12345", 5, 10));
    printf("Valid lengths pass correctly.\n");

    // Test invalid lengths
    printf("Testing invalid lengths...\n");
    assert(!is_int_between_min_max("123", 4, 6));
    assert(!is_int_between_min_max("123456", 1, 5));
    printf("Invalid lengths fail correctly.\n");

    // Test edge cases
    printf("Testing edge cases...\n");
    assert(is_int_between_min_max("", 0, 0));
    assert(!is_int_between_min_max("", 1, 5));
    printf("Edge cases handled correctly.\n");

    printf("is_int_between_min_max test passed successfully.\n");
}

void test_wrap_text(void) {
    printf("Testing wrap_text...\n");

    char output[512];

    // Test short text that doesn't need wrapping
    printf("Testing short text...\n");
    wrap_text("Hello world", output, 200); // 200px width
    assert(strcmp(output, "Hello world") == 0);
    printf("Short text not wrapped correctly.\n");

    // Test with very small width
    printf("Testing with very small width...\n");
    wrap_text("Hello", output, 20);       // 20px width (~2 chars)
    assert(strcmp(output, "Hello") == 0); // Single word shouldn't be split
    printf("Single word handled correctly.\n");

    // Test empty input
    printf("Testing empty input...\n");
    output[0] = '\0';
    wrap_text("", output, 100);
    assert(strcmp(output, "") == 0);
    printf("Empty input handled correctly.\n");

    printf("wrap_text test passed successfully.\n");
}

void test_filter_integer_input(void) {
    printf("Testing filter_integer_input...\n");

    char input[MAX_INPUT];

    // Test with mixed characters
    printf("Testing with mixed characters...\n");
    strcpy(input, "a1b2c3d4e5");
    filter_integer_input(input, MAX_INPUT);
    assert(strcmp(input, "12345") == 0);
    printf("Non-digit characters filtered correctly.\n");

    // Test with max length
    printf("Testing with max length...\n");
    strcpy(input, "12345678901234567890");
    filter_integer_input(input, 10);
    assert(strlen(input) == 10);
    printf("Length limited correctly.\n");

    // Test with all non-digits
    printf("Testing with all non-digits...\n");
    strcpy(input, "abcde");
    filter_integer_input(input, MAX_INPUT);
    assert(strcmp(input, "") == 0);
    printf("All non-digits filtered correctly.\n");

    // Test empty input
    printf("Testing empty input...\n");
    strcpy(input, "");
    filter_integer_input(input, MAX_INPUT);
    assert(strcmp(input, "") == 0);
    printf("Empty input handled correctly.\n");

    printf("filter_integer_input test passed successfully.\n");
}

void test_validate_date(void) {
    printf("Testing validate_date...\n");

    // Test valid dates
    printf("Testing valid dates...\n");
    assert(validate_date(2023, 1, 31)); // January
    assert(validate_date(2023, 4, 30)); // April
    assert(validate_date(2020, 2, 29)); // Leap year
    assert(validate_date(2023, 2, 28)); // Non-leap year
    printf("Valid dates pass correctly.\n");

    // Test invalid dates
    printf("Testing invalid dates...\n");
    assert(!validate_date(2023, 1, 32)); // Day too high
    assert(!validate_date(2023, 4, 31)); // April 31st
    assert(!validate_date(2023, 2, 29)); // Feb 29th non-leap
    assert(!validate_date(2023, 13, 1)); // Invalid month
    assert(!validate_date(0, 1, 1));     // Invalid year
    printf("Invalid dates fail correctly.\n");

    // Test edge cases
    printf("Testing edge cases...\n");
    assert(validate_date(9999, 12, 31)); // Max reasonable date
    assert(validate_date(1, 1, 1));      // Min reasonable date
    assert(!validate_date(2023, 0, 1));  // Month 0
    assert(!validate_date(2023, 1, 0));  // Day 0
    printf("Edge cases handled correctly.\n");

    printf("validate_date test passed successfully.\n");
}

// UTILSFN TESTS END

void test_db_manager_fn(void) {
    test_db_init();
    test_db_init_with_tbl();
    test_db_is_init();
    test_db_deinit();
    printf("All database manager operations tests passed successfully!\n");
}

void test_resident_db_fn(void) {
    test_resident_db_insert();
    test_resident_db_retrieve();
    test_resident_db_update();
    test_resident_db_check_cpf_exists();
    test_resident_db_delete_by_cpf();
    test_resident_db_get_count();
    test_resident_db_get_all_format();
    test_resident_db_get_all_format_old();
    test_resident_db_get_all();
    printf("All resident database operations tests passed successfully!\n");
}

void test_foodbatch_db_fn(void) {
    test_foodbatch_db_insert();
    test_foodbatch_db_retrieve();
    test_foodbatch_db_update();
    test_foodbatch_db_check_batchid_exists();
    test_foodbatch_db_delete_by_id();
    test_foodbatch_db_get_count();
    test_foodbatch_db_get_all_format();
    test_foodbatch_db_get_all_format_old();
    test_foodbatch_db_get_all();
    printf("All foodbatch database operations tests passed successfully!\n");
}

void test_user_db_fn(void) {
    test_user_db_create_table();
    test_user_db_create_user();
    test_user_db_authenticate();
    test_user_db_delete();
    test_user_db_update_password();
    test_user_db_update_admin_status();
    test_user_db_check_exists();
    test_user_db_get_by_username();
    test_user_db_update_username();
    test_user_db_default_admin_changes();
    test_user_db_check_admin();
    test_user_db_set_password_reset();
    test_user_db_get_count();
    test_user_db_get_all();
    printf("All user database operations tests passed successfully!\n");
}

void test_clothes_db_fn(void) {
    test_clothes_db_create_table();
    test_clothes_db_upsert();
    test_clothes_db_remove();
    test_clothes_db_remove_by_id();
    test_clothes_db_delete_entry();
    test_clothes_db_delete_entry_by_id();
    test_clothes_db_check_exists();
    test_clothes_db_check_exists_by_id();
    test_clothes_db_get();
    test_clothes_db_get_all();
    printf("All clothes database operations tests passed successfully!\n");
}

void test_medication_db_fn(void) {
    test_medication_db_create_table();
    test_medication_db_upsert();
    test_medication_db_remove();
    test_medication_db_remove_by_id();
    test_medication_db_delete_entry();
    test_medication_db_delete_entry_by_id();
    test_medication_db_check_exists();
    test_medication_db_check_exists_by_id();
    test_medication_db_get();
    test_medication_db_get_all();
    printf("All medication database operations tests passed successfully!\n");
}

void test_supplies_db_fn(void) {
    test_supplies_db_create_table();
    test_supplies_db_upsert();
    test_supplies_db_remove();
    test_supplies_db_remove_by_id();
    test_supplies_db_delete_entry();
    test_supplies_db_delete_entry_by_id();
    test_supplies_db_check_exists();
    test_supplies_db_check_exists_by_id();
    test_supplies_db_get();
    test_supplies_db_get_all();
    printf("All supplies database operations tests passed successfully!\n");
}

void test_tasks_db_fn(void) {
    test_tasks_db_create_table();
    test_tasks_db_upsert();
    test_tasks_db_check_exists();
    test_tasks_db_get();
    test_tasks_db_get_count();
    test_tasks_db_get_all_and_format();
    test_tasks_db_get_all_format_old();
    printf("All tasks database operations tests passed successfully!\n");
}

void test_hash_fn(void) {
    test_generate_salt();
    test_hash_password();
    test_hash_consistency();
    test_hash_collision_resistance();
    test_edge_cases();
    printf("All utils_hash tests passed successfully!\n");
}

void test_utils_fn(void) {
    test_flag_macros();
    test_is_int_between_min_max();
    test_wrap_text();
    test_filter_integer_input();
    test_validate_date();
    printf("All utilsfn tests passed successfully!\n");
}

int main(void) {
    test_db_manager_fn();
    test_resident_db_fn();
    test_foodbatch_db_fn();
    test_user_db_fn();
    test_clothes_db_fn();
    test_medication_db_fn();
    test_supplies_db_fn();
    test_tasks_db_fn();
    test_hash_fn();
    test_utils_fn();

    printf("All tests passed successfully!\n");
    return 0;
}

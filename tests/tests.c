/**
 * @file tests.c
 * @brief Unit testing for everything that can be unit tested
 */

/** Including raylib + raygui with #define RAYGUI_IMPLEMENTATION
 * As makefile link all src/ files expect main.c for the tests directory
 * It won't see the RAYGUI_IMPLEMENTATION and will complain
 */
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
#include "utils_hash.h"
#include "utilsfn.h"

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

void test_resident_db_get_count() {
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

void test_resident_db_get_all_format() {
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
        "Truncated output (first %llu bytes):\n%.*s\n",
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

void test_resident_db_get_all_format_old() {
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

void test_resident_db_get_all() {
    const char *test_resident_filename = "test_resident_db.db";
    database test_resident_db;
    db_init_with_tbl(&test_resident_db, test_resident_filename, resident_db_create_table);

    setup_cleanup(test_resident_filename, &test_resident_db);

    printf("Testing foodbatch_db_get_all...\n");

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

void test_foodbatch_db_get_all() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    printf("Testing foodbatch_db_get_all...\n");

    // Create several test foodbatches
    printf("Creating test foodbatches...\n");
    foodbatch_db_insert(&test_foodbatch_db, 0, "Milk", 10, true, "2025-12-20", 2);
    foodbatch_db_insert(&test_foodbatch_db, 1, "Rice", 5, false, "0001-01-01", 3);
    foodbatch_db_insert(&test_foodbatch_db, 2, "Beans", 10, false, "0001-01-01", 1);
    foodbatch_db_insert(&test_foodbatch_db, 3, "Tomatoes", 20, true, "2025-05-10", 1);

    // Call get_all (this primarily tests that it doesn't crash)
    printf("Calling foodbatch_db_get_all...\n");
    int rc = foodbatch_db_get_all(&test_foodbatch_db);
    assert(rc == SQLITE_OK);
    printf("foodbatch_db_get_all executed successfully.\n");

    teardown_cleanup();

    printf("foodbatch_db_get_all test passed successfully.\n");
}

void test_foodbatch_db_get_count() {
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
    int rc = foodbatch_db_insert(&test_foodbatch_db, 1, "Milk", 10, true, "2023-12-31", 2.0);
    assert(rc == SQLITE_OK);

    // Verify count is now 1
    printf("Verifying count after insertion...\n");
    count = foodbatch_db_get_count(&test_foodbatch_db);
    assert(count == 1);
    printf("Count correct after insertion (1).\n");

    // Add multiple batches
    printf("Adding multiple food batches...\n");
    foodbatch_db_insert(&test_foodbatch_db, 2, "Bread", 5, true, "2023-11-30", 1.0);
    foodbatch_db_insert(&test_foodbatch_db, 3, "Rice", 20, false, "2024-12-31", 0.5);

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

void test_foodbatch_db_get_all_format() {
    const char *test_foodbatch_filename = "test_foodbatch_db.db";
    database test_foodbatch_db;
    db_init_with_tbl(&test_foodbatch_db, test_foodbatch_filename, foodbatch_db_create_table);

    setup_cleanup(test_foodbatch_filename, &test_foodbatch_db);

    // Test empty database
    printf("Testing format on empty database...\n");
    char buffer[4096];
    int written = foodbatch_db_get_all_format(&test_foodbatch_db, buffer, sizeof(buffer));
    assert(written > 0);
    printf("Empty database format output:\n%s\n", buffer);

    // Add test data
    printf("Adding test food batches...\n");
    foodbatch_db_insert(&test_foodbatch_db, 1, "Milk", 10, true, "2023-12-31", 2.0);
    foodbatch_db_insert(&test_foodbatch_db, 2, "Bread", 5, true, "2023-11-30", 1.0);
    foodbatch_db_insert(&test_foodbatch_db, 3, "Rice", 20, false, "2024-12-31", 0.5);

    // Test with sufficient buffer
    printf("Testing format with sufficient buffer...\n");
    written = foodbatch_db_get_all_format(&test_foodbatch_db, buffer, sizeof(buffer));
    int exact_bytes = written;
    printf("Formatted output with sufficient buffer:\n%s\n", buffer);

    // Verify the format contains expected elements
    assert(strstr(buffer, "BatchId") != NULL);
    assert(strstr(buffer, "Name") != NULL);
    assert(strstr(buffer, "Quantity") != NULL);
    assert(strstr(buffer, "Perishable") != NULL);
    assert(strstr(buffer, "Expiration date") != NULL);
    assert(strstr(buffer, "Daily Rate") != NULL);
    assert(strstr(buffer, "Milk") != NULL);
    assert(strstr(buffer, "Bread") != NULL);
    assert(strstr(buffer, "Rice") != NULL);
    printf("Format contains all expected elements.\n");

    // Test with small buffer (should truncate)
    printf("Testing format with small buffer...\n");
    char small_buffer[100];
    written = foodbatch_db_get_all_format(&test_foodbatch_db, small_buffer, sizeof(small_buffer));
    assert(written == -1);
    printf("Format correctly detected buffer overflow (returned -1).\n");
    printf(
        "Truncated output (first %llu bytes):\n%.*s\n",
        sizeof(small_buffer),
        (int)sizeof(small_buffer),
        small_buffer
    );

    // Test with exact buffer size
    printf("Testing format with exact buffer size...\n");
    written = exact_bytes;
    written = foodbatch_db_get_all_format(&test_foodbatch_db, buffer, written + 1);
    assert(written > 0);
    printf("Format with exact buffer size successful.\n");

    teardown_cleanup();

    printf("foodbatch_db_get_all_format test passed successfully.\n");
}

void test_foodbatch_db_get_all_format_old() {
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
    foodbatch_db_insert(&test_foodbatch_db, 1, "Milk", 10, true, "2023-12-31", 2.0);
    foodbatch_db_insert(&test_foodbatch_db, 2, "Bread", 5, true, "2023-11-30", 1.0);
    foodbatch_db_insert(&test_foodbatch_db, 3, "Rice", 20, false, "2024-12-31", 0.5);
    foodbatch_db_insert(&test_foodbatch_db, 4, "Pasta", 15, false, "2025-01-31", 0.3);
    foodbatch_db_insert(&test_foodbatch_db, 5, "Cheese", 8, true, "2023-10-15", 0.8);

    // Test with multiple records
    printf("Testing format with multiple records...\n");
    result = foodbatch_db_get_all_format_old(&test_foodbatch_db);
    assert(result != NULL);
    printf("Formatted output with multiple records:\n%s\n", result);

    // Verify the format contains expected elements
    assert(strstr(result, "BatchId") != NULL);
    assert(strstr(result, "Name") != NULL);
    assert(strstr(result, "Quantity") != NULL);
    assert(strstr(result, "Perishable") != NULL);
    assert(strstr(result, "Expiration date") != NULL);
    assert(strstr(result, "Daily Rate") != NULL);
    assert(strstr(result, "Milk") != NULL);
    assert(strstr(result, "Bread") != NULL);
    assert(strstr(result, "Rice") != NULL);
    assert(strstr(result, "Pasta") != NULL);
    assert(strstr(result, "Cheese") != NULL);
    printf("Format contains all expected elements.\n");

    free(result);

    teardown_cleanup();

    printf("foodbatch_db_get_all_format_old test passed successfully.\n");
}

// TEST DB FOODBATCH END

// TEST DB USER START

void test_user_db_create_table() {
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
    int rc = user_db_create_user(&test_user_db, "testuser", false, true);
    assert(rc == SQLITE_OK);
    printf("Table structure is correct.\n");

    teardown_cleanup();

    printf("user_db_create_table test passed successfully.\n");
}

void test_user_db_create_user() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_create_user...\n");

    // Test creating a regular user
    printf("Creating regular user 'user1'...\n");
    int rc = user_db_create_user(&test_user_db, "user1", false, true);
    assert(rc == SQLITE_OK);
    printf("Regular user created successfully.\n");

    // Test creating an admin user
    printf("Creating admin user 'admin2'...\n");
    rc = user_db_create_user(&test_user_db, "admin2", true, false);
    assert(rc == SQLITE_OK);
    printf("Admin user created successfully.\n");

    // Test creating duplicate user
    printf("Attempting to create duplicate user 'user1'...\n");
    rc = user_db_create_user(&test_user_db, "user1", false, true);
    assert(rc == SQLITE_CONSTRAINT);
    printf("Duplicate user creation failed as expected.\n");

    teardown_cleanup();

    printf("user_db_create_user test passed successfully.\n");
}

void test_user_db_authenticate() {
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
    int rc = user_db_create_user(&test_user_db, username, false, true);
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

void test_user_db_delete() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_delete...\n");

    // Create a test user
    const char *username = "user_to_delete";
    printf("Creating user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, false, true);
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

void test_user_db_update_password() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_update_password...\n");

    // Create a test user
    const char *username = "password_test_user";
    printf("Creating user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, false, true);
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

void test_user_db_update_admin_status() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    setup_cleanup(test_userdb_filename, &test_user_db);

    // Create a test user (non-admin)
    const char *username = "admin_test_user";
    printf("Creating non-admin user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, false, true);
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

void test_user_db_check_exists() {
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
    const char *username = "existence_test_user";
    printf("Creating user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, false, true);
    assert(rc == SQLITE_OK);

    // Check for existing user
    printf("Checking for existing user...\n");
    exists = user_db_check_exists(&test_user_db, username);
    assert(exists);
    printf("Existing user correctly found.\n");

    teardown_cleanup();

    printf("user_db_check_exists test passed successfully.\n");
}

void test_user_db_get_by_username() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_get_by_username...\n");

    // Create a test user with known properties
    const char *username = "get_test_user";
    bool is_admin = true;
    bool reset_password = false;
    printf("Creating test user '%s'...\n", username);
    int rc = user_db_create_user(&test_user_db, username, is_admin, reset_password);
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
    assert(user_data.is_admin == is_admin);
    assert(user_data.reset_password == reset_password);
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

void test_user_db_change_username() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_change_username...\n");

    // Create a test user with known properties
    const char *old_username = "old_username";
    const char *new_username = "new_username";
    bool is_admin = false;
    bool reset_password = true;
    printf("Creating test user '%s'...\n", old_username);
    int rc = user_db_create_user(&test_user_db, old_username, is_admin, reset_password);
    assert(rc == SQLITE_OK);

    // Set password
    const char *password = "testpassword";
    printf("Setting password...\n");
    rc = user_db_update_password(&test_user_db, old_username, password);
    assert(rc == SQLITE_OK);
    // password reset set as false in the database

    // Change username
    printf("Changing username from '%s' to '%s'...\n", old_username, new_username);
    rc = user_db_change_username(&test_user_db, old_username, new_username);
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
    rc = user_db_change_username(&test_user_db, new_username, "admin"); // admin exists
    assert(rc == SQLITE_CONSTRAINT);
    printf("Change to existing username failed as expected.\n");

    // Try to change non-existent user
    printf("Attempting to change non-existent user...\n");
    rc = user_db_change_username(&test_user_db, "nonexistent", "whatever");
    assert(rc == SQLITE_NOTFOUND);
    printf("Change of non-existent user failed as expected.\n");

    teardown_cleanup();

    printf("user_db_change_username test passed successfully.\n");
}

void test_user_db_default_admin_changes() {
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
    rc = user_db_change_username(&test_user_db, "admin", "newusername");
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

void test_user_db_check_admin() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Checking if default admin is admin...\n");
    bool is_admin = user_db_check_admin_status(&test_user_db, "admin");

    assert(is_admin == true);
    printf("Default admin is correctly an admin.\n");

    printf("Checking if a newly created admin user is admin...\n");
    user_db_create_user(&test_user_db, "testadmin", true, true);

    is_admin = user_db_check_admin_status(&test_user_db, "testadmin");
    assert(is_admin == true);
    printf("Newly create admin user is correctly an admin.\n");

    printf("Checking if a newly created non-admin user is admin...\n");
    user_db_create_user(&test_user_db, "test_nonadmin", false, true);

    is_admin = user_db_check_admin_status(&test_user_db, "test_nonadmin");
    assert(is_admin == false);
    printf("Newly create non-admin user is correctly not an admin.\n");

    teardown_cleanup();
}

void test_user_db_get_all() {
    const char *test_userdb_filename = "test_user_db.db";
    database test_user_db;
    db_init_with_tbl(&test_user_db, test_userdb_filename, user_db_create_table);

    setup_cleanup(test_userdb_filename, &test_user_db);

    printf("Testing user_db_get_all...\n");

    // Create several test users
    printf("Creating test users...\n");
    user_db_create_user(&test_user_db, "user1", false, true);
    user_db_create_user(&test_user_db, "user2", false, false);
    user_db_create_user(&test_user_db, "admin1", true, true);
    user_db_create_user(&test_user_db, "admin2", true, false);

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

// UTILS_HASH TESTS

#include "user.h"
#include "utils_hash.h"

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
void test_generate_salt() {
    printf("Testing generate_salt...\n");

    // Test normal operation
    printf("Testing normal salt generation...\n");
    char salt[SALT_LEN + 1];
    generate_salt(salt, SALT_LEN);

    // Verify length and null termination
    assert(count_non_null_bytes(salt, SALT_LEN) == SALT_LEN);
    assert(salt[SALT_LEN ] == '\0');

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

void test_hash_password() {
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
        assert(isxdigit(hash[i]));
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

void test_hash_consistency() {
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

void test_hash_collision_resistance() {
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

void test_edge_cases() {
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

void test_flag_macros() {
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

void test_is_int_between_min_max() {
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

void test_wrap_text() {
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

void test_filter_integer_input() {
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

void test_validate_date() {
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

int main() {
    test_resident_db_insert();
    test_resident_db_retrieve();
    test_resident_db_update();
    test_resident_db_check_cpf_exists();
    test_resident_db_delete_by_cpf();
    test_resident_db_get_count();
    test_resident_db_get_all_format();
    test_resident_db_get_all_format_old();
    test_resident_db_get_all();

    test_foodbatch_db_insert();
    test_foodbatch_db_retrieve();
    test_foodbatch_db_update();
    test_foodbatch_db_check_batchid_exists();
    test_foodbatch_db_delete_by_id();
    test_foodbatch_db_get_count();
    test_foodbatch_db_get_all_format();
    test_foodbatch_db_get_all_format_old();
    test_foodbatch_db_get_all();

    test_user_db_create_table();
    test_user_db_create_user();
    test_user_db_authenticate();
    test_user_db_delete();
    test_user_db_update_password();
    test_user_db_update_admin_status();
    test_user_db_check_exists();
    test_user_db_get_by_username();
    test_user_db_change_username();
    test_user_db_default_admin_changes();
    test_user_db_check_admin();
    test_user_db_get_all();

    test_generate_salt();
    test_hash_password();
    test_hash_consistency();
    test_hash_collision_resistance();
    test_edge_cases();

    test_flag_macros();
    test_is_int_between_min_max();
    test_wrap_text();
    test_filter_integer_input();
    test_validate_date();

    return 0;
}
/**
 * @file main.c
 * @brief Shelter Management System - Main Application Entry
 *
 * Core application file managing:
 * - System initialization
 * - Main event loop
 * - State management
 * - Resource cleanup
 *
 * @note Uses Raylib+Raygui for graphics and GUI
 * @note Uses SQLite3 for database operations
 * @note Uses OpenSSL for crypto
 */

#include <external/raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raylib/raygui.h>
#include <external/sqlite3/sqlite3.h>

#include <stdio.h>

#include "CONSTANTS.h"
#include "globals.h"
#include "utilsfn.h"

#include "app_state.h"
#include "db/db_manager.h"
#include "db/foodbatch_db.h"
#include "db/resident_db.h"
#include "db/user_db.h"
#include "error_handling.h"
#include "foodbatch.h"
#include "resident.h"
#include "ui/ui_create_user.h"
#include "ui/ui_food.h"
#include "ui/ui_login.h"
#include "ui/ui_main_menu.h"
#include "ui/ui_persistent.h"
#include "ui/ui_resident.h"
#include "ui_elements/button.h"
#include "ui_elements/checkbox.h"
#include "ui_elements/dropdownbox.h"
#include "ui_elements/floatbox.h"
#include "ui_elements/intbox.h"
#include "ui_elements/textbox.h"
#include "user.h"

/**
  * @brief Application entry point
  *
  * Initializes and manages the Shelter Management System lifecycle:
  * 1. Graphics system initialization
  * 2. Database connections setup
  * 3. UI systems initialization
  * 4. Main application loop
  * 5. Resource cleanup
  *
  * @return int Application exit code:
  *         - EXIT_SUCCESS (0) on normal termination
  *         - ERROR_OPENING_DB on database failures
  *         - EXIT_FAILURE on critical errors
  *
  * @note Uses goto for centralized error cleanup
  * @warning All database connections must be properly closed before exit
  */
int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    int return_code = EXIT_SUCCESS;

    // Configure and create application window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "Shelter Management");

    if (!IsWindowReady()) {
        fprintf(stderr, "Error opening graphics window.\n");
        return_code = EXIT_FAILURE;
        goto cleanup;
    }

    // Configure GUI defaults
    GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);
    SetTargetFPS(60);

    // Database connections
    database resident_db = { 0 };  ///< Resident records database
    database foodbatch_db = { 0 }; ///< Food inventory database
    database user_db = { 0 };      ///< User accounts database

    // Initialize databases with tables
    if (db_init_with_tbl(&resident_db, "resident_db.db", resident_db_create_table) != SQLITE_OK) {
        fprintf(stderr, "Error opening resident db.\n");
        return_code = ERROR_OPENING_DB;
        goto cleanup;
    }

    if (db_init_with_tbl(&foodbatch_db, "foodbatch_db.db", foodbatch_db_create_table) != SQLITE_OK) {
        fprintf(stderr, "Error opening foodbatch db.\n");
        return_code = ERROR_OPENING_DB;
        goto cleanup;
    }

    if (db_init_with_tbl(&user_db, "user_db.db", user_db_create_table) != SQLITE_OK) {
        fprintf(stderr, "Error opening user db.\n");
        return_code = ERROR_OPENING_DB;
        goto cleanup;
    }

    /**
     * Initializing a non-admin user here just for testing ui permissions
     */
    const char *test_user = "test";
    user_db_delete(&user_db, test_user); // delete first if already created
    user_db_create_user(&user_db, test_user, false, true);

    // Application state tracking
    struct user current_user = { 0 };            ///< Currently logged in user
    enum error_code error = NO_ERROR;            ///< Application error state
    enum app_state app_state = STATE_LOGIN_MENU; ///< Current application screen

    // Initialize UI systems
    struct ui_login ui_login = { 0 }; ///< Login screen interface
    ui_login_init(&ui_login, &current_user);

    struct ui_main_menu ui_main_menu = { 0 }; ///< Main menu interface
    ui_main_menu_init(&ui_main_menu, &current_user);

    struct ui_resident ui_resident = { 0 }; ///< Resident management interface
    ui_resident_init(&ui_resident);

    struct ui_food ui_food = { 0 }; ///< Food management interface
    ui_food_init(&ui_food);

    struct ui_create_user ui_create_user = { 0 }; ///< Create new user interface
    ui_create_user_init(&ui_create_user);

    struct ui_persistent ui_persistent = { 0 }; ///< Persistent UI elements
    ui_persistent_init(&ui_persistent);

    // Main application loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------

        // Handle window resize events
        if (IsWindowResized()) {
            update_window_size(GetScreenWidth(), GetScreenHeight());
            ui_login.base.update_positions(&ui_login.base);
            ui_resident.base.update_positions(&ui_resident.base);
            ui_food.base.update_positions(&ui_food.base);
            ui_persistent_updt_pos(&ui_persistent);
        }

        // Updating the current style selector
        ui_persistent_updt(&ui_persistent);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // State machine for screen rendering
        switch (app_state) {
        case STATE_LOGIN_MENU:
            ui_login.base.render(&ui_login.base, &app_state, &error, &user_db);
            break;
        case STATE_MAIN_MENU:
            ui_main_menu.base.render(&ui_main_menu.base, &app_state, &error, &user_db);
            break;
        case STATE_REGISTER_RESIDENT:
            ui_resident.base.render(&ui_resident.base, &app_state, &error, &resident_db);
            break;
        case STATE_REGISTER_FOOD:
            ui_food.base.render(&ui_food, &app_state, &error, &foodbatch_db);
            break;
        case STATE_CREATE_USER:
            ui_create_user_draw(&ui_create_user, &app_state, &error, &user_db);
        default:
            break;
        }

        // Draw persistent UI elements (visible in all states)
        ui_persistent_draw(&ui_persistent, &current_user, &app_state);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-initialization
    //--------------------------------------------------------------------------------------
cleanup:
    // Cleanup database connections if initialized
    if (db_is_init(&resident_db)) {
        db_deinit(&resident_db);
    }

    if (db_is_init(&foodbatch_db)) {
        db_deinit(&foodbatch_db);
    }

    if (db_is_init(&user_db)) {
        db_deinit(&user_db);
    }

    // Close graphics window
    CloseWindow();
    //--------------------------------------------------------------------------------------
    return return_code;
}
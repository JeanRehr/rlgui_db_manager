/*
 * how many people there are in total inside the database
 * calculate the average of food that 1 resident needs
 * how many babies (for baby specific stuff) and woman (for woman specific stuff)
 * if anyone needs any pharmacies
 * total food that is in stock and total that needs to be maintained for 1 month
 * calculate for how long the current food will last
 * how long will take for the food to spoil in case of milk, eggs...
 * register people that enters
 * register food that enters
 * register people that gets out
 * register food that gets out
 * two databases, one for food and one for people
 * one screen for inserting food, one screen for inserting people
 */

#include <external/raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raylib/raygui.h>
#include <external/sqlite3/sqlite3.h>

// raygui embedded styles
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE 13 // NOTE: Included light style
#include "styles/style_amber.h"
#include "styles/style_ashes.h" // raygui style: ashes
#include "styles/style_bluish.h" // raygui style: bluish
#include "styles/style_candy.h" // raygui style: candy
#include "styles/style_cherry.h" // raygui style: cherry
#include "styles/style_cyber.h" // raygui style: cyber
#include "styles/style_dark.h" // raygui style: dark
#include "styles/style_enefete.h" // raygui style: enefete
#include "styles/style_jungle.h" // raygui style: jungle
#include "styles/style_lavanda.h" // raygui style: lavanda
#include "styles/style_sunny.h" // raygui style: sunny
#include "styles/style_terminal.h" // raygui style: terminal

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
#include "food.h"
#include "resident.h"
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

int active_style = 8; // Set default style to dark
int prev_active_style = 7;

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    int return_code = EXIT_SUCCESS;
    database resident_db = { 0 };
    database foodbatch_db = { 0 };
    database user_db = { 0 };

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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "Shelter Management");

    if (!IsWindowReady()) {
        fprintf(stderr, "Error opening graphics window.\n");
        return_code = EXIT_FAILURE;
        goto cleanup;
    }

    GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

    SetTargetFPS(60);

    struct ui_login ui_login;

    ui_login_init(&ui_login);

    struct ui_main_menu ui_main_menu;

    ui_main_menu_init(&ui_main_menu);

    struct ui_resident ui_resident;

    ui_resident_init(&ui_resident);

    struct ui_food ui_food;

    ui_food_init(&ui_food);

    struct ui_persistent ui_persistent;

    ui_persistent_init(&ui_persistent);

    // Setting the initial state of the app
    struct user current_user = { 0 };
    enum error_code error = NO_ERROR;
    enum app_state app_state = STATE_LOGIN_MENU;

    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

        if (IsWindowResized()) {
            update_window_size(GetScreenWidth(), GetScreenHeight());
            ui_login_updt_pos(&ui_login);
            ui_resident_updt_pos(&ui_resident);
            ui_food_updt_pos(&ui_food);
            ui_persistent_updt_pos(&ui_persistent);
        }

        if (active_style != prev_active_style) {
            // Reset to default internal style
            // NOTE: Required to unload any previously loaded font texture
            GuiLoadStyleDefault();

            switch (active_style) {
            case 1:
                GuiLoadStyleJungle();
                break;
            case 2:
                GuiLoadStyleCandy();
                break;
            case 3:
                GuiLoadStyleLavanda();
                break;
            case 4:
                GuiLoadStyleCyber();
                break;
            case 5:
                GuiLoadStyleTerminal();
                break;
            case 6:
                GuiLoadStyleAshes();
                break;
            case 7:
                GuiLoadStyleBluish();
                break;
            case 8:
                GuiLoadStyleDark();
                break;
            case 9:
                GuiLoadStyleCherry();
                break;
            case 10:
                GuiLoadStyleSunny();
                break;
            case 11:
                GuiLoadStyleEnefete();
                break;
            case 12:
                GuiLoadStyleAmber();
                break;
            default:
                break;
            }

            prev_active_style = active_style;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        switch (app_state) {
        case STATE_LOGIN_MENU:
            ui_login_draw(&ui_login, &app_state, &error, &user_db, &current_user);
            break;
        case STATE_MAIN_MENU:
            ui_main_menu_draw(&ui_main_menu, &app_state, &error);
            break;

        case STATE_REGISTER_RESIDENT:
            ui_resident_draw(&ui_resident, &app_state, &error, &resident_db);
            break;

        case STATE_REGISTER_FOOD:
            ui_food_draw(&ui_food, &app_state, &error, &foodbatch_db);
            break;

        default:
            break;
        }

        ui_persistent_draw(&ui_persistent, &current_user, &app_state, &active_style);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-initialization
    //--------------------------------------------------------------------------------------
cleanup:
    if (db_is_init(&resident_db)) {
        db_deinit(&resident_db);
    }

    if (db_is_init(&foodbatch_db)) {
        db_deinit(&foodbatch_db);
    }

    if (db_is_init(&user_db)) {
        db_deinit(&user_db);
    }

    CloseWindow();
    //--------------------------------------------------------------------------------------
    return return_code;
}
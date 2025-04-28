#include <external/raylib/raygui.h>

#include "ui/ui_main_menu.h"
#include "globals.h"
#include "utilsfn.h"

/**
 * @brief Private function to handle the button actions.
 * Simple actions like only changing state are made directly here.
 * 
 * @param ui Pointer to ui_main_menu struct to handle button action
 * @param state Pointer to the state of the app
 * @param error Pointer to the error code
 * @param user_db Pointer to the user_db database
 * @param current_user Pointer to the current_user to check if it has permission agains the database
 */
static void handle_button_actions(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

static void handle_create_user_button(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

static void show_warning_messages(struct ui_main_menu *ui, enum error_code *error);

void ui_main_menu_init(struct ui_main_menu *ui) {
    ui->reg_resident_butn = button_init((Rectangle) { 100, 100, 200, 50 }, "Manage Persons");

    ui->reg_food_butn = button_init(
        (Rectangle) { ui->reg_resident_butn.bounds.x, ui->reg_resident_butn.bounds.y + 100, 200, 50 },
        "Manage Food"
    );

    ui->create_user_butn = button_init(
        (Rectangle) { ui->reg_food_butn.bounds.x, ui->reg_food_butn.bounds.y + 100, 200, 50 },
        "Create User"
    );

    ui->flag = 0;
}

void ui_main_menu_draw(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
) {
    // Start draw UI elements

    // Start button actions
    handle_button_actions(ui, state, error, user_db, current_user);

    // Start show warning/error boxes
    show_warning_messages(ui, error);
}

static void handle_button_actions(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
) {
    if (button_draw_updt(&ui->reg_resident_butn)) {
        *state = STATE_REGISTER_RESIDENT;
        return;
    }

    if (button_draw_updt(&ui->reg_food_butn)) {
        *state = STATE_REGISTER_FOOD;
        return;
    }

    if (button_draw_updt(&ui->create_user_butn)) {
        handle_create_user_button(ui, state, error, user_db, current_user);
        /** @todo user db validation
         * Can do the validation two ways:
         * - check if current_user.is_admin; or
         * - check with the database if current username is_admin on the database
         * 
         * first option can easily be tempered with cheating tools
         */
    }
}

static void handle_create_user_button(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
) {
    // Clear any previous flag
    CLEAR_FLAG(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN);

    bool is_admin = user_db_check_admin_status(user_db, current_user->username);
    if (!is_admin) {
        SET_FLAG(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN);
        return;
    }

    *state = STATE_CREATE_USER;
}

static void show_warning_messages(struct ui_main_menu *ui, enum error_code *error) {
    const char *message = NULL;
    enum main_menu_screen_flags flag_to_clear = 0;

    if (IS_FLAG_SET(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN)) {
        message = "User is not an admin.";
        flag_to_clear = FLAG_MAIN_MENU_WARN_NOT_ADMIN;
    }

    if (message) {
        const char *buttons = "OK";

        int result = GuiMessageBox(
            (Rectangle) { window_width / 2 - 150, window_height / 2 - 50, 300, 100 },
            "#191#Warning!",
            message,
            buttons
        );

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

void ui_main_menu_updt_pos(struct ui_main_menu *ui) {}
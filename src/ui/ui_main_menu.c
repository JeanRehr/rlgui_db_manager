#include <external/raylib/raygui.h>

#include "db/user_db.h"
#include "globals.h"
#include "ui/ui_main_menu.h"
#include "utilsfn.h"

// Forward declarations

static void handle_manage_resident_button(enum app_state *state);

static void handle_manage_food_button(enum app_state *state);

static void handle_create_user_button(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

// Public functions

void ui_main_menu_init(struct ui_main_menu *ui, struct user *current_user) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Main Menu");
    // Override methods
    ui->base.render = ui_main_menu_render;
    ui->base.handle_buttons = ui_main_menu_handle_buttons;
    ui->base.handle_warning_msg = ui_main_menu_handle_warning_msg;

    // Initialize ui specific fields

    ui->current_user = current_user;

    ui->reg_resident_butn = button_init((Rectangle) { 100, 100, 200, 50 }, "Manage Residents");

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

void ui_main_menu_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db) {
    struct ui_main_menu *ui = (struct ui_main_menu *)base;

    ui->base.handle_buttons(&ui->base, state, error, user_db);

    ui->base.handle_warning_msg(&ui->base, state, error, user_db);
}

void ui_main_menu_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_main_menu *ui = (struct ui_main_menu *)base;

    if (button_draw_updt(&ui->reg_resident_butn)) {
        handle_manage_resident_button(state);
        return;
    }

    if (button_draw_updt(&ui->reg_food_butn)) {
        handle_manage_food_button(state);
        return;
    }

    if (button_draw_updt(&ui->create_user_butn)) {
        handle_create_user_button(ui, state, error, user_db, ui->current_user);
        return;
    }
}

void ui_main_menu_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    // Explicitly mark as unused
    (void)state;
    (void)error;
    (void)user_db;

    struct ui_main_menu *ui = (struct ui_main_menu *)base;

    const char *message = NULL;
    enum main_menu_screen_flags flag_to_clear = 0;

    if (IS_FLAG_SET(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN)) {
        message = "User is not an admin.";
        flag_to_clear = FLAG_MAIN_MENU_WARN_NOT_ADMIN;
    }

    if (message) {
        const char *buttons = "OK";

        int result = GuiMessageBox(
            (Rectangle) { window_width / 2 - 150, window_height / 2 - 50, 300, 150 },
            "#191#Warning!",
            message,
            buttons
        );

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

// Private (static) functions

static void handle_manage_resident_button(enum app_state *state) {
    *state = STATE_REGISTER_RESIDENT;
}

static void handle_manage_food_button(enum app_state *state) {
    *state = STATE_REGISTER_FOOD;
}

static void handle_create_user_button(
    struct ui_main_menu *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
) {
    (void)error; // Explicitly mark as unused

    // Clear any previous flag
    CLEAR_FLAG(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN);

    bool is_admin = user_db_check_admin_status(user_db, current_user->username);
    if (!is_admin) {
        SET_FLAG(&ui->flag, FLAG_MAIN_MENU_WARN_NOT_ADMIN);
        return;
    }

    *state = STATE_CREATE_USER;
}
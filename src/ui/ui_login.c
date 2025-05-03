#include <external/raylib/raygui.h>

#include <stdio.h>
#include <string.h>

#include "ui/ui_login.h"

#include "db/user_db.h"
#include "globals.h"
#include "utilsfn.h"

// Forward declarations

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_login_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_UPDT_PASS,
};

// Info for the database operation based on the type
struct ui_login_db_action_info {
    enum ui_login_db_action_type type;
    union {
        struct {
            const char *username;
            const char *new_password;
        } updt_pass;
    };
};

static void handle_login_button(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

/**
 * @internal
 * @brief Processes database actions triggered by warning messages
 * 
 * Handles password updates and other DB operations that may be requested
 * through warning message dialogs.
 * 
 * @param ui Login UI context
 * @param state Application state to modify
 * @param error Error code to set if operation fails
 * @param user_db Database connection
 * @param current_user User session to update
 * @param action Database action to perform with parameters
 */
static void process_db_action_in_warning(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user,
    struct ui_login_db_action_info *action
);

// Public functions

void ui_login_init(struct ui_login *ui, struct user *current_user) {
    // Initialize base
    ui_base_init_defaults(&ui->base);
    // Override methods
    ui->base.render = ui_login_render;
    ui->base.handle_buttons = ui_login_handle_buttons;
    ui->base.handle_warning_msg = ui_login_handle_warning_msg;
    ui->base.update_positions = ui_login_updt_pos;
    ui->base.clear_fields = ui_login_clear_fields;

    // Initialize screen specific fields
    ui->current_user = current_user;

    ui->tb_username =
        textbox_init((Rectangle) { window_width / 2 - 150, window_height / 2 - 15, 300, 30 }, "Username:");

    ui->tbs_password = textboxsecret_init(
        (Rectangle
        ) { ui->tb_username.bounds.x, ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2), 300, 30 },
        "Password:"
    );

    ui->butn_login = button_init(
        (Rectangle) { ui->tbs_password.bounds.x + (ui->tbs_password.bounds.width / 2) - 50,
                      ui->tbs_password.bounds.y + (ui->tbs_password.bounds.height * 2),
                      100,
                      30 },
        "Login"
    );

    ui->flag = 0;
}

void ui_login_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db) {
    struct ui_login *ui = (struct ui_login *)base;

    // Draw UI elements
    textbox_draw(&ui->tb_username);
    textboxsecret_draw(&ui->tbs_password);

    // Handle and draw buttons
    ui->base.handle_buttons(&ui->base, state, error, user_db);

    ui->base.handle_warning_msg(&ui->base, state, error, user_db);

    if (IS_FLAG_SET(&ui->flag, FLAG_LOGIN_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_LOGIN_DONE);
    }
}

void ui_login_handle_buttons(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db) {
    struct ui_login *ui = (struct ui_login *)base;
    if (button_draw_updt(&ui->butn_login) || IsKeyPressed(KEY_ENTER)) {
        handle_login_button(ui, state, error, user_db, ui->current_user);
    }
}

void ui_login_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_login *ui = (struct ui_login *)base;

    const char *message = NULL;
    enum login_screen_flags flag_to_clear = 0;
    struct ui_login_db_action_info action = { DB_ACTION_NONE };

    if (IS_FLAG_SET(&ui->flag, FLAG_USERNAME_EMPTY)) {
        message = "Username must not be empty.";
        flag_to_clear = FLAG_USERNAME_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_PASSWD_EMPTY)) {
        message = "Password must not be empty";
        flag_to_clear = FLAG_PASSWD_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_USER_NOT_EXISTS | FLAG_WRONG_PASSWD)) {
        message = "Invalid username or password";
        flag_to_clear = FLAG_USER_NOT_EXISTS | FLAG_WRONG_PASSWD;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_PASSWD_RESET)) {
        message = "New password must be set.\nThe password you entered will become\nyour new password.";
        flag_to_clear = FLAG_PASSWD_RESET;
        action.type = DB_ACTION_UPDT_PASS;
        action.updt_pass.username = ui->tb_username.input;
        action.updt_pass.new_password = ui->tbs_password.input;
    } else if (*error != NO_ERROR) {
        message = "Database error!";
        *error = NO_ERROR; // Clear error after showing
    }

    if (message) {
        const char *buttons = (action.type != DB_ACTION_NONE) ? "Yes;No" : "Ok";
        int result = GuiMessageBox(
            (Rectangle) { window_width / 2 - 150, window_height / 2 - 50, 300, 150 },
            "#191#Warning!",
            message,
            buttons
        );

        if (result == 1 && action.type != DB_ACTION_NONE) {
            process_db_action_in_warning(ui, state, error, user_db, ui->current_user, &action);
            if (*error == ERROR_UPDATE_DB) {
                message = "Failed to update password. Please try again.";
                GuiMessageBox(
                    (Rectangle) { window_width / 2 - 150, window_height / 2 - 50, 300, 150 },
                    "#191#Warning!",
                    message,
                    "OK"
                );
            }
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear); // Clear the flag
        }
    }
}

void ui_login_updt_pos(struct ui_base *base) {
    struct ui_login *ui = (struct ui_login *)base;

    ui->tb_username.bounds.x = window_width / 2 - 150;
    ui->tb_username.bounds.y = window_height / 2 - 15;

    ui->tbs_password.bounds.x = ui->tb_username.bounds.x;
    ui->tbs_password.bounds.y = ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2);

    ui->butn_login.bounds.x = ui->tbs_password.bounds.x + (ui->tbs_password.bounds.width / 2) - 50;
    ui->butn_login.bounds.y = ui->tbs_password.bounds.y + (ui->tbs_password.bounds.height * 2);
}

void ui_login_clear_fields(struct ui_base *base) {
    struct ui_login *ui = (struct ui_login *)base;
    ui->tb_username.input[0] = '\0';
    memset(ui->tbs_password.input, 0, sizeof(ui->tbs_password.input)); // More secure
}

// Private (static) functions

static void handle_login_button(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
) {
    // Clear previous flags
    CLEAR_FLAG(
        &ui->flag,
        FLAG_USERNAME_EMPTY | FLAG_PASSWD_EMPTY | FLAG_USER_NOT_EXISTS | FLAG_WRONG_PASSWD | FLAG_PASSWD_RESET
    );

    // Validate inputs
    if (ui->tb_username.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_USERNAME_EMPTY);
        return;
    }

    if (ui->tbs_password.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_PASSWD_EMPTY);
        return;
    }

    if (!user_db_check_exists(user_db, ui->tb_username.input)) {
        SET_FLAG(&ui->flag, FLAG_USER_NOT_EXISTS);
        return;
    }

    // Perform authentication
    enum auth_result result = user_db_authenticate(user_db, ui->tb_username.input, ui->tbs_password.input);

    switch (result) {
    case AUTH_NEED_PASSWORD_RESET:
        SET_FLAG(&ui->flag, FLAG_PASSWD_RESET);
        break;

    case AUTH_SUCCESS: {
        // without braces, the following happens:
        // warning: a label can only be part of a statement and a declaration is not a statement [-Wpedantic]
        int rc = user_db_get_by_username(user_db, ui->tb_username.input, current_user);
        if (rc != SQLITE_OK && rc != SQLITE_NOTFOUND) {
            *error = ERROR_DB_STMT;
            break;
        }
        *state = STATE_MAIN_MENU;
        SET_FLAG(&ui->flag, FLAG_LOGIN_DONE);
        break;
    }

    case AUTH_FAILURE:
    default:
        SET_FLAG(&ui->flag, FLAG_WRONG_PASSWD);
        break;
    }
}

static void process_db_action_in_warning(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user,
    struct ui_login_db_action_info *action
) {
    switch (action->type) {
    case DB_ACTION_UPDT_PASS:
        if (user_db_update_password(user_db, action->updt_pass.username, action->updt_pass.new_password) != SQLITE_OK) {
            *error = ERROR_UPDATE_DB;
            break;
        }
        user_db_get_by_username(user_db, ui->tb_username.input, current_user);
        *state = STATE_MAIN_MENU;
        SET_FLAG(&ui->flag, FLAG_LOGIN_DONE);
        break;

    case DB_ACTION_NONE:
    default:
        break;
    }
}
/**
 * @file ui_create_user.c
 * @brief Creation of users implementation
 */
#include "ui/ui_create_user.h"

#include <stdlib.h>

#include <external/raylib/raygui.h>

#include "db/user_db.h"
#include "globals.h"
#include "utilsfn.h"

/* Forward declarations */

static void ui_create_user_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

static void ui_create_user_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

static void ui_create_user_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

static void ui_create_user_update_positions(struct ui_base *base);

static void ui_create_user_clear_fields(struct ui_base *base);

static void ui_create_user_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_user_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_DELETE,
};

// Info for the database operation based on the type
struct ui_user_db_action_info {
    enum ui_user_db_action_type type;
    union {
        struct {
            const char *username;
        } delete;
    };
};

static void process_db_action_in_warning(
    struct ui_create_user *ui,
    enum error_code *error,
    struct ui_user_db_action_info *action,
    database *user_db
);

static void draw_user_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_create_user *ui, enum app_state *state);

static void handle_create_user_button(struct ui_create_user *ui, enum error_code *error, database *user_db);

static void handle_reset_password_button(struct ui_create_user *ui, enum error_code *error, database *user_db);

static void handle_update_admin_status(struct ui_create_user *ui, enum error_code *error, database *user_db);

static void handle_delete_button(struct ui_create_user *ui, database *user_db);

static void handle_get_all_button(struct ui_create_user *ui, database *user_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_create_user_init(struct ui_create_user *ui) {
    /* Initialize base to default no-op */
    ui_base_init_defaults(&ui->base, "UI Create User");

    /* Override methods */
    ui->base.render = ui_create_user_render;
    ui->base.handle_buttons = ui_create_user_handle_buttons;
    ui->base.handle_warning_msg = ui_create_user_handle_warning_msg;
    ui->base.update_positions = ui_create_user_update_positions;
    ui->base.clear_fields = ui_create_user_clear_fields;
    ui->base.cleanup = ui_create_user_cleanup;

    /* UI Specific fields */
    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_username = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 300, 30 },
        "Username:"
    );

    ui->tbi_cpf = textboxint_init(
        (Rectangle) { 20, ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2), 300, 30 },
        "CPF:"
    );

    ui->tbi_phone_number = textboxint_init(
        (Rectangle) { 20, ui->tbi_cpf.bounds.y + (ui->tbi_cpf.bounds.height * 2), 300, 30 },
        "Phone Number:"
    );

    ui->cb_is_admin = checkbox_init(
        (Rectangle) { 20, ui->tbi_phone_number.bounds.y + (ui->tbi_phone_number.bounds.height * 2), 20, 20 },
        "Admin:"
    );

    ui->butn_create_user = button_init((Rectangle) { 20, window_height - 60, 100, 30 }, "Create User");

    ui->butn_reset_password = button_init(
        (Rectangle
        ) { ui->butn_create_user.bounds.x + ui->butn_create_user.bounds.width + 10, window_height - 60, 100, 30 },
        "Reset Password"
    );

    ui->butn_update_adm_stat = button_init(
        (Rectangle
        ) { ui->butn_reset_password.bounds.x + ui->butn_reset_password.bounds.width + 10, window_height - 60, 100, 30 },
        "Update Admin Status"
    );

    ui->butn_delete = button_init(
        (Rectangle) { ui->butn_update_adm_stat.bounds.x + ui->butn_update_adm_stat.bounds.width + 10,
                      window_height - 60,
                      100,
                      30 },
        "Delete User"
    );

    ui->butn_get_all = button_init(
        (Rectangle) { ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, window_height - 60, 100, 30 },
        "Retrieve Users"
    );

    ui->table_view = scrollpanel_init(
        (Rectangle) { ui->tb_username.bounds.x + ui->tb_username.bounds.width + 10,
                      10,
                      window_width - (ui->tb_username.bounds.x + ui->tb_username.bounds.width + 20 + 110),
                      window_height - 100 },
        "Database view",
        (Rectangle) { 0, 0, 0, 0 }
    );
    ui->table_content = NULL;

    ui->flag = 0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Renders create user screen and handles interactions
 *
 * @implements ui_base.render
 *
 * @param base Must cast to ui_create_user*
 * @param state Modified on screen transition
 * @param error Set on database failures
 * @param user_db User database connection
 *
 * @warning Immediate-mode rendering (draws and handles input in one pass)
 *
 */
static void ui_create_user_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_create_user *ui = (struct ui_create_user *)base;

    textbox_draw(&ui->tb_username);
    textboxint_draw(&ui->tbi_cpf);
    textboxint_draw(&ui->tbi_phone_number);
    checkbox_draw(&ui->cb_is_admin);

    scrollpanel_draw(&ui->table_view, draw_user_table_content, ui->table_content);

    ui->base.handle_buttons(&ui->base, state, error, user_db);

    ui->base.handle_warning_msg(&ui->base, state, error, user_db);

    if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE);
    }
}

/**
 * @brief Handle button drawing and logic.
 *
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 * @param state Pointer to application state (modified on success)
 * @param error Pointer to error tracking variable
 * @param resident_db Pointer to resident database connection
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_create_user_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_create_user *ui = (struct ui_create_user *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_create_user)) {
        handle_create_user_button(ui, error, user_db);
        return;
    }

    if (button_draw_updt(&ui->butn_reset_password)) {
        handle_reset_password_button(ui, error, user_db);
        return;
    }

    if (button_draw_updt(&ui->butn_update_adm_stat)) {
        handle_update_admin_status(ui, error, user_db);
        return;
    }

    if (button_draw_updt(&ui->butn_delete)) {
        handle_delete_button(ui, user_db);
        return;
    }

    if (button_draw_updt(&ui->butn_get_all)) {
        handle_get_all_button(ui, user_db);
        return;
    }

    return;
}

/**
 * @brief Manages warning message display and response handling.
 *
 * @implements ui_base.handle_warning_msg
 *
 * Shows appropriate warning messages (e.g., deletions),
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 * @param state Pointer to application state (ui_resident does not modify this, but the interface needs this parameter)
 * @param error Pointer to error tracking variable
 * @param resident_db Pointer to resident database connection
 *
 * @warning Should be called through the base interface, may trigger database operations
 *
 */
static void ui_create_user_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_create_user *ui = (struct ui_create_user *)base;

    (void)state; // Explicitly mark as unused

    const char *message = NULL;
    enum create_user_screen_flags flag_to_clear = 0;
    struct ui_user_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY)) {
        message = "Username must not be empty.";
        flag_to_clear = FLAG_CREATE_USER_USERNAME_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_USERNAME_ALREADY_EXISTS)) {
        message = "This username already exists.";
        flag_to_clear = FLAG_CREATE_USER_USERNAME_ALREADY_EXISTS;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_USERNAME_NOT_EXISTS)) {
        message = "Username does not exists.";
        flag_to_clear = FLAG_CREATE_USER_USERNAME_NOT_EXISTS;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_CPF_EMPTY)) {
        message = "CPF must not be empty.";
        flag_to_clear = FLAG_CREATE_USER_CPF_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_CPF_EXISTS)) {
        message = "CPF already exists.";
        flag_to_clear = FLAG_CREATE_USER_CPF_EXISTS;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_CPF_NOT_VALID)) {
        message = "CPF is not valid, must be 11 digits.";
        flag_to_clear = FLAG_CREATE_USER_CPF_NOT_VALID;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_PHONE_NUMBER_WRONG)) {
        message = "Phone number not expected format.\nMust be exactly 13 digits or nothing.";
        flag_to_clear = FLAG_CREATE_USER_PHONE_NUMBER_WRONG;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_CONFIRM_DELETE)) {
        message = "Are you sure you want to\ndelete this user?";
        flag_to_clear = FLAG_CREATE_USER_CONFIRM_DELETE;
        action.type = DB_ACTION_DELETE;
        action.delete.username = ui->tb_username.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CREATE_USER_ADMIN_TEMPER)) {
        message = "Default admin cannot be tempered with.\nCan only reset its password.";
        flag_to_clear = FLAG_CREATE_USER_ADMIN_TEMPER;
    } else if (*error != NO_ERROR) {
        message = "Database error. Try Again";
        *error = NO_ERROR; // Clear error after showing
    }

    if (message) {
        const char *buttons = (action.type != DB_ACTION_NONE) ? "Yes;No" : "OK";
        int result = GuiMessageBox(
            (Rectangle) { window_width / 2 - 150, window_height / 2 - 50, 300, 150 },
            "#191#Warning!",
            message,
            buttons
        );

        if (result == 1 && action.type != DB_ACTION_NONE) {
            process_db_action_in_warning(ui, error, &action, user_db);
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

/**
 * @brief Updates element positions for window resizing
 *
 * Adjusts UI element positions based on current window dimensions.
 * Should be called when the window is resized.
 *
 * @implements ui_base.update_positions
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 *
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_create_user_update_positions(struct ui_base *base) {
    struct ui_create_user *ui = (struct ui_create_user *)base;
    ui->butn_create_user.bounds.y = window_height - 60;
    ui->butn_reset_password.bounds.y = window_height - 60;
    ui->butn_update_adm_stat.bounds.y = window_height - 60;
    ui->butn_delete.bounds.y = window_height - 60;
    ui->butn_get_all.bounds.y = window_height - 60;
    ui->table_view.panel_bounds.width = window_width
        - (ui->tb_username.bounds.x + ui->tb_username.bounds.width + 20 + /* +100 for styler offset */ 110);
    ui->table_view.panel_bounds.height = window_height - 100;
}

/**
 * @brief Clear fields of the ui_create_user
 *
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_create_user_clear_fields(struct ui_base *base) {
    struct ui_create_user *ui = (struct ui_create_user *)base;

    ui->cb_is_admin.checked = false;
    ui->tb_username.input[0] = '\0';
    ui->tbi_cpf.input[0] = '\0';
    ui->tbi_phone_number.input[0] = '\0';
}

/**
 * @brief Cleanup of any memory allocated by ui create user
 *
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_create_user_cleanup(struct ui_base *base) {
    struct ui_create_user *ui = (struct ui_create_user *)base;
    (void)ui;
}

/** @} */

/* ======================= INTERNAL HELPERS ======================= */

static void process_db_action_in_warning(
    struct ui_create_user *ui,
    enum error_code *error,
    struct ui_user_db_action_info *action,
    database *user_db
) {
    switch (action->type) {
    case DB_ACTION_DELETE: { ///< -Werror -pedantic a label can only be part of a statement and a declaration is not a statement
        int rc = user_db_delete(user_db, action->delete.username);
        if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_CREATE_USER_ADMIN_TEMPER);
            break;
        } else if (rc != SQLITE_OK) {
            *error = ERROR_DELETE_DB;
            break;
        }

        SET_FLAG(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE);
        break;
    }

    case DB_ACTION_NONE:
    default:
        break;
    }
}

/**
 * @internal
 * @brief Draws the table content of the database
 *
 * @note This is a callback to be used in the scrollpanel_draw
 *
 */
static void draw_user_table_content(Rectangle bounds, char *data) {
    GuiLabel(bounds, data ? data : "No data");
}

/**
 * @internal
 * @brief Private function to handle going back to the main menu.
 *        Any freeing of memory should be done here if necessary, set it back to null for further use.
 *
 * @param ui Pointer to ui_resident struct to handle button action
 * @param state Pointer to the state of the app
 *
 */
static void handle_back_button(struct ui_create_user *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);

    *state = STATE_MAIN_MENU;
}

static void handle_create_user_button(struct ui_create_user *ui, enum error_code *error, database *user_db) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_CREATE_USER_USERNAME_EMPTY | FLAG_CREATE_USER_USERNAME_ALREADY_EXISTS | FLAG_CREATE_USER_CPF_NOT_VALID
            | FLAG_CREATE_USER_CPF_EMPTY | FLAG_CREATE_USER_CPF_EXISTS | FLAG_CREATE_USER_PHONE_NUMBER_WRONG
    );

    if (ui->tb_username.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY);
        return;
    }

    if (ui->tbi_cpf.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_CPF_EMPTY);
        return;
    }

    if (!is_int_between_min_max(ui->tbi_cpf.input, 11, 11)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_CPF_NOT_VALID);
        return;
    }

    if (ui->tbi_phone_number.input[0] != '\0') {
        if (!is_int_between_min_max(ui->tbi_phone_number.input, 13, 13)) {
            SET_FLAG(&ui->flag, FLAG_CREATE_USER_PHONE_NUMBER_WRONG);
            return;
        }
    }

    // Check if username exists
    if (user_db_check_exists(user_db, ui->tb_username.input)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_ALREADY_EXISTS);
        return;
    }

    // Check if CPF exists
    if (user_db_check_cpf_exists(user_db, ui->tbi_cpf.input)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_CPF_EXISTS);
        return;
    }

    if (user_db_create_user(
            user_db,
            ui->tb_username.input,
            ui->tbi_cpf.input,
            ui->tbi_phone_number.input,
            ui->cb_is_admin.checked
        )
        != SQLITE_OK)
    {
        *error = ERROR_INSERT_DB;
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE);
    *error = NO_ERROR;
}

static void handle_reset_password_button(struct ui_create_user *ui, enum error_code *error, database *user_db) {
    CLEAR_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY | FLAG_CREATE_USER_USERNAME_NOT_EXISTS);

    if (ui->tb_username.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY);
        return;
    }

    if (!user_db_check_exists(user_db, ui->tb_username.input)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_NOT_EXISTS);
        return;
    }

    if (user_db_set_reset_password(user_db, ui->tb_username.input) != SQLITE_OK) {
        *error = ERROR_INSERT_DB;
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE);
    *error = NO_ERROR;
}

static void handle_update_admin_status(struct ui_create_user *ui, enum error_code *error, database *user_db) {
    CLEAR_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY | FLAG_CREATE_USER_USERNAME_NOT_EXISTS);

    if (ui->tb_username.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY);
        return;
    }

    if (!user_db_check_exists(user_db, ui->tb_username.input)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_NOT_EXISTS);
        return;
    }

    int rc = user_db_update_admin_status(user_db, ui->tb_username.input, ui->cb_is_admin.checked);

    if (rc == SQLITE_CONSTRAINT) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_ADMIN_TEMPER);
        return;
    }

    if (rc != SQLITE_OK) {
        *error = ERROR_INSERT_DB;
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CREATE_USER_OPERATION_DONE);
    *error = NO_ERROR;
}

static void handle_delete_button(struct ui_create_user *ui, database *user_db) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_CREATE_USER_CONFIRM_DELETE | FLAG_CREATE_USER_USERNAME_EMPTY | FLAG_CREATE_USER_USERNAME_NOT_EXISTS
    );

    if (ui->tb_username.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_EMPTY);
        return;
    }

    if (!user_db_check_exists(user_db, ui->tb_username.input)) {
        SET_FLAG(&ui->flag, FLAG_CREATE_USER_USERNAME_NOT_EXISTS);
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CREATE_USER_CONFIRM_DELETE);
}

static void handle_get_all_button(struct ui_create_user *ui, database *user_db) {
    if (ui->table_content) {
        free(ui->table_content); // Free old data before getting new data
        ui->table_content = NULL;
    }

    user_db_get_all(user_db);
    return;
}

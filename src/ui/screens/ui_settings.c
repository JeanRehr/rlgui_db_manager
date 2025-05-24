/**
 * @file ui_settings.c
 * @brief Settings screen implementation
 */
#include "ui/screens/ui_settings.h"

#include <external/raylib/raygui.h>

#include <external/raylib/raygui.h>

// raygui embedded styles
// Embedded a monospace font in them as well.
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE 15 // NOTE: Included light style
#include "styles/amber.h"           // raygui styleL amber
#include "styles/ashes.h"           // raygui style: ashes
#include "styles/bluish.h"          // raygui style: bluish
#include "styles/candy.h"           // raygui style: candy
#include "styles/cherry.h"          // raygui style: cherry
#include "styles/cyber.h"           // raygui style: cyber
#include "styles/dark.h"            // raygui style: dark
#include "styles/enefete.h"         // raygui style: enefete
#include "styles/genesis.h"         // raygui style: genesis
#include "styles/jungle.h"          // raygui style: jungle
#include "styles/lavanda.h"         // raygui style: lavanda
#include "styles/light.h"           // raygui style: default
#include "styles/rltech.h"          // raygui style: rltech
#include "styles/sunny.h"           // raygui style: sunny
#include "styles/terminal.h"        // raygui style: terminal

#include "db/user_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_settings_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db);

static void ui_settings_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

static void ui_settings_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
);

static void ui_settings_update_positions(struct ui_base *base);

static void ui_settings_clear_fields(struct ui_base *base);

static void detect_styler_changes(struct ui_settings *ui);

static void draw_current_user_info_panel(struct ui_settings *ui);

static void handle_back_button(enum app_state *state);

static void handle_submit_button(struct ui_settings *ui, enum error_code *error, database *user_db);

static void handle_reset_password_button(struct ui_settings *ui, enum error_code *error, database *user_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_settings_init(struct ui_settings *ui, struct user *current_user) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "ui_settings.c");

    // Override methods
    ui->base.render = ui_settings_render;
    ui->base.handle_buttons = ui_settings_handle_buttons;
    ui->base.handle_warning_msg = ui_settings_handle_warning_msg;
    ui->base.update_positions = ui_settings_update_positions;
    ui->base.clear_fields = ui_settings_clear_fields;

    // UI user specific fields

    ui->current_user = current_user;

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_new_username = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 300, 30 },
        "New Username:"
    );

    ui->tbi_new_phone_number = textboxint_init(
        (Rectangle) { 20, ui->tb_new_username.bounds.y + (ui->tb_new_username.bounds.height * 2), 300, 30 },
        "New Phone Number:"
    );

    ui->tbi_new_cpf = textboxint_init(
        (Rectangle) { 20, ui->tbi_new_phone_number.bounds.y + (ui->tbi_new_phone_number.bounds.height * 2), 300, 30 },
        "New CPF:"
    );

    ui->ddb_style_options = dropdownbox_init(
        (Rectangle) { window_width - 110, 30, 100, 30 },
        "Amber;Ashes;Bluish;Candy;Cherry;Cyber;Dark;Enefete;Genesis;Jungle;Lavanda;Light;RLTech;Sunny;Terminal",
        "Style:"
    );

    ui->ddb_style_options.active_option = 8;                         ///< Currently active GUI style (default: Genesis)
    ui->prev_active_style = ui->ddb_style_options.active_option - 1; ///< Previously active style for change detection

    ui->butn_submit = button_init((Rectangle) { 20, window_height - 60, 100, 30 }, "Submit");

    ui->butn_reset_password = button_init(
        (Rectangle) { ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30 },
        "Reset Password"
    );

    // Only set the bounds of the panel, draw everything inside based on it
    ui->panel_bounds =
        (Rectangle) { ui->tb_new_username.bounds.x + ui->tb_new_username.bounds.width + 10, 10, 300, 250 };

    ui->flag = 0;

    // Set a default theme in init
    // Reset to default internal style
    // NOTE: Required to unload any previously loaded font texture
    GuiLoadStyleDefault();
    GuiLoadStyleGenesis();
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Renders user screen and handles interactions
 * 
 * @implements ui_base.render
 * 
 * @param base Must cast to ui_settings*
 * @param state Modified on screen transition
 * @param error Set on database failures
 * @param user_db user database connection
 * 
 * @warning Immediate-mode rendering (draws and handles input in one pass)
 * 
 */
static void ui_settings_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *user_db) {
    struct ui_settings *ui = (struct ui_settings *)base;

    // Start draw UI elements

    dropdownbox_draw(&ui->ddb_style_options);
    textbox_draw(&ui->tb_new_username);
    textboxint_draw(&ui->tbi_new_phone_number);
    textboxint_draw(&ui->tbi_new_cpf);

    // End draw UI elements

    // Style selector
    detect_styler_changes(ui);

    // Panel info
    draw_current_user_info_panel(ui);

    // Handle button actions
    ui->base.handle_buttons(&ui->base, state, error, user_db);

    // Show warning/error messages
    ui->base.handle_warning_msg(&ui->base, state, error, user_db);

    // Clear fields after successful operation
    if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_SETTINGS_OPERATION_DONE);
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
 * @param user_db Pointer to user database connection
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_settings_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_settings *ui = (struct ui_settings *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(state);
        return;
    }

    if (button_draw_updt(&ui->butn_submit)) {
        handle_submit_button(ui, error, user_db);
        return;
    }

    if (button_draw_updt(&ui->butn_reset_password)) {
        handle_reset_password_button(ui, error, user_db);
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
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 *
 * @warning Should be called through the base interface, may trigger database operations
 *
 */
static void ui_settings_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *user_db
) {
    struct ui_settings *ui = (struct ui_settings *)base;
    (void)state;
    (void)user_db;

    const char *message = NULL;
    enum settings_screen_flags flag_to_clear = 0;

    if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_USERNAME_EXISTS)) {
        message = "This username already\nexists.";
        flag_to_clear = FLAG_SETTINGS_USERNAME_EXISTS;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_CPF_EXISTS)) {
        message = "This CPF already exists.";
        flag_to_clear = FLAG_SETTINGS_CPF_EXISTS;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_CPF_NOT_VALID)) {
        message = "CPF must be exactly 11\ndigits.";
        flag_to_clear = FLAG_SETTINGS_CPF_NOT_VALID;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_PHONE_NUMBER_WRONG)) {
        message = "Phone Number must be\nexactly 13 digits.";
        flag_to_clear = FLAG_SETTINGS_PHONE_NUMBER_WRONG;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SETTINGS_ADMIN_TEMPER)) {
        message = "Username of default admin can't be\nchanged. Other fields\nhave been changed.";
        flag_to_clear = FLAG_SETTINGS_ADMIN_TEMPER;
    } else if (*error == ERROR_UPDATE_DB) {
        message = "Database error. Try Again";
        *error = NO_ERROR; // Clear error after showing
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
static void ui_settings_update_positions(struct ui_base *base) {
    struct ui_settings *ui = (struct ui_settings *)base;
    ui->butn_submit.bounds.y = window_height - 60;
    ui->butn_reset_password.bounds.y = window_height - 60;
    ui->ddb_style_options.bounds.x = window_width - 110;
}

/**
 * @brief Clear fields of the ui_settings
 *
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 *
 * @warning Should be called through the base interface
 *
 */
static void ui_settings_clear_fields(struct ui_base *base) {
    struct ui_settings *ui = (struct ui_settings *)base;
    ui->tb_new_username.input[0] = '\0';
    ui->tbi_new_cpf.input[0] = '\0';
    ui->tbi_new_phone_number.input[0] = '\0';
}
/** @} */

/* ======================= INTERNAL HELPERS ======================= */

/**
 * @private
 * @brief Handles GUI style changes from the selector
 * 
 * @param ui Pointer to the ui_settings struct
 *
 */
static void detect_styler_changes(struct ui_settings *ui) {
    // No change detected
    if (ui->ddb_style_options.active_option == ui->prev_active_style) {
        return;
    }

    // Reset to default internal style
    // NOTE: Required to unload any previously loaded font texture
    GuiLoadStyleDefault();

    // Load selected style
    switch (ui->ddb_style_options.active_option) {
    case 0:
        GuiLoadStyleAmber();
        break;
    case 1:
        GuiLoadStyleAshes();
        break;
    case 2:
        GuiLoadStyleBluish();
        break;
    case 3:
        GuiLoadStyleCandy();
        break;
    case 4:
        GuiLoadStyleCherry();
        break;
    case 5:
        GuiLoadStyleCyber();
        break;
    case 6:
        GuiLoadStyleDark();
        break;
    case 7:
        GuiLoadStyleEnefete();
        break;
    case 8:
        GuiLoadStyleGenesis();
        break;
    case 9:
        GuiLoadStyleJungle();
        break;
    case 10:
        GuiLoadStyleLavanda();
        break;
    case 11:
        GuiLoadStyleLight();
        break;
    case 12:
        GuiLoadStyleRLTech();
        break;
    case 13:
        GuiLoadStyleSunny();
        break;
    case 14:
        GuiLoadStyleTerminal();
        break;
    default:
        break;
    }

    ui->prev_active_style = ui->ddb_style_options.active_option;
}

/**
 * @internal
 * @brief Draws the current user info panel
 * 
 */
static void draw_current_user_info_panel(struct ui_settings *ui) {
    GuiPanel(ui->panel_bounds, TextFormat("Current User:"));

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20 },
        TextFormat("Username: %s", ui->current_user->username)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20 },
        TextFormat("CPF: %s", ui->current_user->cpf)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20 },
        TextFormat("Phone Number: %s", ui->current_user->phone_number)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20 },
        TextFormat("Is Admin: %s", ui->current_user->is_admin == true ? "Yes" : "No")
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20 },
        TextFormat("Flagged for password reset: %s", ui->current_user->reset_password == true ? "Yes" : "No")
    );
}

static void handle_back_button(enum app_state *state) {
    *state = STATE_MAIN_MENU;
    return;
}

/**
 * @internal
 * @brief Private function to handle the submit of updated user info into the database.
 * 
 * @param ui Pointer to ui_settings struct to handle button action
 * @param error Pointer to the error code
 * @param user_db Pointer to the user database
 * 
 * @warning New username must be set LAST!
 *
 */
static void handle_submit_button(struct ui_settings *ui, enum error_code *error, database *user_db) {
    // Clear previous flags
    CLEAR_FLAG(
        &ui->flag,
        FLAG_SETTINGS_CPF_NOT_VALID | FLAG_SETTINGS_PHONE_NUMBER_WRONG | FLAG_SETTINGS_USERNAME_EXISTS
            | FLAG_SETTINGS_CPF_EXISTS | FLAG_SETTINGS_ADMIN_TEMPER
    );

    // Validate inputs
    if (ui->tbi_new_cpf.input[0] != '\0' && !is_int_between_min_max(ui->tbi_new_cpf.input, 11, 11)) {
        SET_FLAG(&ui->flag, FLAG_SETTINGS_CPF_NOT_VALID);
        return;
    }

    if (ui->tbi_new_phone_number.input[0] != '\0' && !is_int_between_min_max(ui->tbi_new_phone_number.input, 13, 13)) {
        SET_FLAG(&ui->flag, FLAG_SETTINGS_PHONE_NUMBER_WRONG);
        return;
    }

    if (user_db_check_exists(user_db, ui->tb_new_username.input)) {
        SET_FLAG(&ui->flag, FLAG_SETTINGS_USERNAME_EXISTS);
        return;
    }

    if (user_db_check_cpf_exists(user_db, ui->tbi_new_cpf.input)) {
        SET_FLAG(&ui->flag, FLAG_SETTINGS_CPF_EXISTS);
        return;
    }

    if (ui->tbi_new_cpf.input[0] != '\0') {
        if (user_db_update_cpf(user_db, ui->current_user->username, ui->tbi_new_cpf.input) != SQLITE_OK) {
            *error = ERROR_UPDATE_DB;
            return;
        }
    }

    if (ui->tbi_new_phone_number.input[0] != '\0') {
        if (user_db_update_phone_number(user_db, ui->current_user->username, ui->tbi_new_phone_number.input)
            != SQLITE_OK)
        {
            *error = ERROR_UPDATE_DB;
            return;
        }
    }

    if (ui->tb_new_username.input[0] != '\0') {
        int rc = user_db_update_username(user_db, ui->current_user->username, ui->tb_new_username.input);

        if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_SETTINGS_ADMIN_TEMPER);
            return;
        }

        if (rc != SQLITE_OK) {
            *error = ERROR_INSERT_DB;
            return;
        }
    }

    // Update current user pointer
    // These operations are safe because the buffers are all limited by the app's constants
    if (ui->tbi_new_cpf.input[0] != '\0') {
        strcpy(ui->current_user->cpf, ui->tbi_new_cpf.input);
    }
    if (ui->tbi_new_phone_number.input[0] != '\0') {
        strcpy(ui->current_user->phone_number, ui->tbi_new_phone_number.input);
    }
    if (ui->tb_new_username.input[0] != '\0') {
        strcpy(ui->current_user->username, ui->tb_new_username.input);
    }

    SET_FLAG(&ui->flag, FLAG_SETTINGS_OPERATION_DONE);
    return;
}

static void handle_reset_password_button(struct ui_settings *ui, enum error_code *error, database *user_db) {
    if (user_db_set_reset_password(user_db, ui->current_user->username) != SQLITE_OK) {
        *error = ERROR_INSERT_DB;
        return;
    }
    ui->current_user->reset_password = true;
}

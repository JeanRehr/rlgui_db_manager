/**
 * @file ui_resident.c
 * @brief Resident screen implementation
 */
#include "ui/ui_resident.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <external/raylib/raygui.h>

#include "db/resident_db.h"
#include "globals.h"
#include "utilsfn.h"

/* Forward declarations */

static void ui_resident_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
);

static void ui_resident_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
);

static void ui_resident_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
);

static void ui_resident_update_positions(struct ui_base *base);

static void ui_resident_clear_fields(struct ui_base *base);

static void ui_resident_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_resident_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_UPDATE,
    DB_ACTION_DELETE,
};

// Info for the database operation based on the type
struct ui_resident_db_action_info {
    enum ui_resident_db_action_type type;
    union {
        struct {
            const char *cpf;
            const char *name;
            int age;
            const char *health_status;
            const char *needs;
            bool medical_assistance;
            int gender;
        } update;

        struct {
            const char *cpf;
        } delete;
    };
};

static void process_db_action_in_warning(
    struct ui_resident *ui,
    enum error_code *error,
    struct ui_resident_db_action_info *action,
    database *resident_db
);

static void draw_resident_info_panel(struct ui_resident *ui);

static void draw_resident_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_resident *ui, enum app_state *state);

static void handle_submit_button(struct ui_resident *ui, enum error_code *error, database *resident_db);

static void handle_retrieve_button(struct ui_resident *ui, database *resident_db);

static void handle_delete_button(struct ui_resident *ui, database *resident_db);

static void handle_retrieve_all_button(struct ui_resident *ui, database *resident_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_resident_init(struct ui_resident *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "ui_resident.c");

    // Override methods
    ui->base.render = ui_resident_render;
    ui->base.handle_buttons = ui_resident_handle_buttons;
    ui->base.handle_warning_msg = ui_resident_handle_warning_msg;
    ui->base.update_positions = ui_resident_update_positions;
    ui->base.clear_fields = ui_resident_clear_fields;
    ui->base.cleanup = ui_resident_cleanup;

    // UI Resident specific fields
    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");
    ui->tb_name =
        textbox_init((Rectangle) { 20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 300, 30 }, "Name:");
    ui->tb_cpf =
        textboxint_init((Rectangle) { 20, ui->tb_name.bounds.y + (ui->tb_name.bounds.height * 2), 300, 30 }, "CPF:");
    ui->ib_age =
        intbox_init((Rectangle) { 20, ui->tb_cpf.bounds.y + (ui->tb_cpf.bounds.height * 2), 125, 30 }, "Age:", 0, 120);
    ui->tb_health_status = textbox_init(
        (Rectangle) { 20, ui->ib_age.bounds.y + (ui->ib_age.bounds.height * 2), 300, 30 },
        "Health Status:"
    );
    ui->tb_needs = textbox_init(
        (Rectangle) { 20, ui->tb_health_status.bounds.y + (ui->tb_health_status.bounds.height * 2), 300, 30 },
        "Needs:"
    );

    ui->cb_medical_assistance = checkbox_init(
        (Rectangle) { 20, ui->tb_needs.bounds.y + (ui->tb_needs.bounds.height * 2), 20, 20 },
        "Need Medical Assistance:"
    );

    ui->ddb_gender = dropdownbox_init(
        (Rectangle) { 20, ui->cb_medical_assistance.bounds.y + (ui->cb_medical_assistance.bounds.height * 2), 200, 30 },
        "Other;Male;Female",
        "Gender"
    );

    ui->butn_submit = button_init((Rectangle) { 20, window_height - 60, 100, 30 }, "Submit");
    ui->butn_retrieve = button_init(
        (Rectangle) { ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30 },
        "Retrieve"
    );
    ui->butn_delete = button_init(
        (Rectangle
        ) { ui->butn_retrieve.bounds.x + ui->butn_retrieve.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30 },
        "Delete"
    );
    ui->butn_retrieve_all = button_init(
        (Rectangle) { ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, ui->butn_submit.bounds.y, 0, 30 },
        "Retrieve All"
    );

    // Only set the bounds of the panel, draw everything inside based on it on the draw register resident screen function
    ui->panel_bounds = (Rectangle) { ui->tb_name.bounds.x + ui->tb_name.bounds.width + 10, 10, 300, 250 };

    memset(&ui->resident_retrieved, 0, sizeof(struct resident));

    ui->table_view = scrollpanel_init(
        (Rectangle) { ui->panel_bounds.x + ui->panel_bounds.width + 10,
                      10,
                      window_width - (ui->panel_bounds.x + ui->panel_bounds.width + 20 + 110),
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
 * @brief Renders resident screen and handles interactions
 * 
 * @implements ui_base.render
 * 
 * @param base Must cast to ui_resident*
 * @param state Modified on screen transition
 * @param error Set on database failures
 * @param resident_db Resident database connection
 * 
 * @warning Immediate-mode rendering (draws and handles input in one pass)
 * 
 */
static void ui_resident_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
) {
    struct ui_resident *ui = (struct ui_resident *)base;

    // Start draw UI elements

    textbox_draw(&ui->tb_name);
    textboxint_draw(&ui->tb_cpf);
    intbox_draw(&ui->ib_age);

    dropdownbox_draw(&ui->ddb_gender);

    textbox_draw(&ui->tb_health_status);
    textbox_draw(&ui->tb_needs);

    checkbox_draw(&ui->cb_medical_assistance);

    // End draw UI elements

    // Draw info panel
    draw_resident_info_panel(ui);

    // Draw database content
    scrollpanel_draw(&ui->table_view, draw_resident_table_content, ui->table_content);

    // Handle button actions
    ui->base.handle_buttons(&ui->base, state, error, resident_db);

    // Show warning/error messages
    ui->base.handle_warning_msg(&ui->base, state, error, resident_db);

    // Clear fields after successful operation
    if (IS_FLAG_SET(&ui->flag, FLAG_RESIDENT_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
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
static void ui_resident_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
) {
    struct ui_resident *ui = (struct ui_resident *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_submit)) {
        handle_submit_button(ui, error, resident_db);
        return;
    }

    if (button_draw_updt(&ui->butn_retrieve)) {
        handle_retrieve_button(ui, resident_db);
        return;
    }

    if (button_draw_updt(&ui->butn_delete)) {
        handle_delete_button(ui, resident_db);
        return;
    }

    if (button_draw_updt(&ui->butn_retrieve_all)) {
        handle_retrieve_all_button(ui, resident_db);
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
static void ui_resident_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *resident_db
) {
    struct ui_resident *ui = (struct ui_resident *)base;

    (void)state; // Explicitly mark as unused

    const char *message = NULL;
    enum resident_screen_flags flag_to_clear = 0;
    struct ui_resident_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    if (IS_FLAG_SET(&ui->flag, FLAG_INPUT_CPF_EMPTY)) {
        message = "CPF must not be empty.";
        flag_to_clear = FLAG_INPUT_CPF_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_VALID)) {
        message = "CPF must be 11 digits.";
        flag_to_clear = FLAG_CPF_NOT_VALID;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_FOUND)) {
        message = "CPF not found.";
        flag_to_clear = FLAG_CPF_NOT_FOUND;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_EXISTS)) {
        message = "CPF already exists. Update?";
        flag_to_clear = FLAG_CPF_EXISTS;
        action.type = DB_ACTION_UPDATE;
        action.update.cpf = ui->tb_cpf.input;
        action.update.name = ui->tb_name.input;
        action.update.age = ui->ib_age.input;
        action.update.health_status = ui->tb_health_status.input;
        action.update.needs = ui->tb_needs.input;
        action.update.medical_assistance = ui->cb_medical_assistance.checked;
        action.update.gender = ui->ddb_gender.active_option;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CONFIRM_RESIDENT_DELETE)) {
        message = "Are you sure you want to\ndelete this resident?";
        flag_to_clear = FLAG_CONFIRM_RESIDENT_DELETE;
        action.type = DB_ACTION_DELETE;
        action.delete.cpf = ui->tb_cpf.input;
    } else if (*error == ERROR_INSERT_DB || *error == ERROR_UPDATE_DB) {
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
            process_db_action_in_warning(ui, error, &action, resident_db);
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
static void ui_resident_update_positions(struct ui_base *base) {
    struct ui_resident *ui = (struct ui_resident *)base;

    ui->butn_submit.bounds.y = window_height - 60;
    ui->butn_retrieve.bounds.y = ui->butn_submit.bounds.y;
    ui->butn_delete.bounds.y = ui->butn_submit.bounds.y;
    ui->butn_retrieve_all.bounds.y = ui->butn_submit.bounds.y;
    ui->table_view.panel_bounds.width =
        window_width - (ui->panel_bounds.x + ui->panel_bounds.width + 20 + /* +100 for styler offset */ 110);
    ui->table_view.panel_bounds.height = window_height - 100;
}

/**
 * @brief Clear fields of the ui_resident
 * 
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 * 
 * @warning Should be called through the base interface
 * 
 */
static void ui_resident_clear_fields(struct ui_base *base) {
    struct ui_resident *ui = (struct ui_resident *)base;

    ui->tb_name.input[0] = '\0';
    ui->tb_cpf.input[0] = '\0';
    ui->ib_age.input = 0;
    ui->tb_health_status.input[0] = '\0';
    ui->tb_needs.input[0] = '\0';
    ui->cb_medical_assistance.checked = false;
    ui->ddb_gender.active_option = 0;
}

/**
 * @brief Cleanup of any memory allocated by ui resident
 * 
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to any ui*)
 * 
 * @warning Should be called through the base interface
 * 
 */
static void ui_resident_cleanup(struct ui_base *base) {
    struct ui_resident *ui = (struct ui_resident *)base;

    if (ui->table_content) {
        free(ui->table_content);
        ui->table_content = NULL; // Prevent double-free
    }
}
/** @} */

/* ======================= INTERNAL HELPERS ======================= */

/**
 * @internal
 * @brief Processes database actions triggered by warning messages
 * 
 * Handles resident updates, deletion and other DB operations that may be requested
 * through warning message dialogs.
 * 
 * @param ui UI context
 * @param error Error code to set if operation fails
 * @param action Database action to perform with parameters
 * @param resident_db Database connection
 * 
 */
static void process_db_action_in_warning(
    struct ui_resident *ui,
    enum error_code *error,
    struct ui_resident_db_action_info *action,
    database *resident_db
) {
    switch (action->type) {
    case DB_ACTION_UPDATE:
        if (resident_db_update(
                resident_db,
                action->update.cpf,
                action->update.name,
                action->update.age,
                action->update.health_status,
                action->update.needs,
                action->update.medical_assistance,
                action->update.gender
            )
            != SQLITE_OK)
        {
            *error = ERROR_UPDATE_DB;
            break;
        }
        SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
        break;

    case DB_ACTION_DELETE:
        if (resident_db_delete_by_cpf(resident_db, action->delete.cpf) != SQLITE_OK) {
            *error = ERROR_DELETE_DB;
            break;
        }
        SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
        break;

    case DB_ACTION_NONE:
    default:
        break;
    }
}

/**
 * @internal
 * @brief Draws the resident info panel (health/needs summary)
 * 
 */
static void draw_resident_info_panel(struct ui_resident *ui) {
    GuiPanel(ui->panel_bounds, TextFormat("CPF info retrieved: %s", ui->resident_retrieved.cpf));
    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20 },
        TextFormat("Name: %s", ui->resident_retrieved.name)
    );
    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20 },
        TextFormat("Age: %d", ui->resident_retrieved.age)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20 },
        TextFormat("Health Status: %.15s...", ui->resident_retrieved.health_status)
    );
    GuiLabel((Rectangle) { ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20 }, "?");

    if (CheckCollisionPointRec(
            GetMousePosition(),
            (Rectangle) { ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20 }
        ))
    {
        SET_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
    } else {
        CLEAR_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
    }

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20 },
        TextFormat("Needs: %.15s...", ui->resident_retrieved.needs)
    );
    GuiLabel((Rectangle) { ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20 }, "?");

    if (CheckCollisionPointRec(
            GetMousePosition(),
            (Rectangle) { ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20 }
        ))
    {
        SET_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
    } else {
        CLEAR_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
    }

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20 },
        TextFormat("Need Medical Assistance: %s", ui->resident_retrieved.medical_assistance == true ? "True" : "False")
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 180, 280, 20 },
        TextFormat(
            "Gender: %s",
            ui->resident_retrieved.gender == GENDER_OTHER    ? "Other" :
                ui->resident_retrieved.gender == GENDER_MALE ? "Male" :
                                                               "Female"
        )
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 210, 280, 20 },
        TextFormat("Entry Date: %s", ui->resident_retrieved.entry_date)
    );

    if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_HEALTH)) {
        int dyn_max_input = (int)(MAX_INPUT / 0.9);
        char wrapped_text[dyn_max_input];
        wrap_text(ui->resident_retrieved.health_status, wrapped_text, ui->panel_bounds.width);
        GuiMessageBox(
            (Rectangle) { ui->panel_bounds.x, window_height / 2 - 50, ui->panel_bounds.width, 300 },
            "#191#Full Health Status",
            wrapped_text,
            ""
        );
    }

    if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_NEEDS)) {
        int dyn_max_input = (int)(MAX_INPUT / 0.9);
        char wrapped_text[dyn_max_input];
        wrap_text(ui->resident_retrieved.needs, wrapped_text, ui->panel_bounds.width);
        GuiMessageBox(
            (Rectangle) { ui->panel_bounds.x, window_height / 2, ui->panel_bounds.width, 300 },
            "#191#Full Needs",
            wrapped_text,
            ""
        );
    }
}

/**
 * @internal
 * @brief Draws the table content of the database
 * 
 * @note This is a callback to be used in the scrollpanel_draw
 * 
 */
static void draw_resident_table_content(Rectangle bounds, char *data) {
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
static void handle_back_button(struct ui_resident *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);

    *state = STATE_MAIN_MENU;
    return;
}

/**
 * @internal
 * @brief Private function to handle the submit of a resident into the database.
 * 
 * @param ui Pointer to ui_resident struct to handle button action
 * @param error Pointer to the error code
 * @param resident_db Pointer to the resident database
 *
 */
static void handle_submit_button(struct ui_resident *ui, enum error_code *error, database *resident_db) {
    // Clear previous flags
    CLEAR_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY | FLAG_CPF_NOT_VALID | FLAG_CPF_EXISTS);

    // Validate inputs
    if (ui->tb_cpf.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
        return;
    }

    if (!is_int_between_min_max(ui->tb_cpf.input, 11, 11)) {
        SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
        return;
    }

    // Check if CPF exists
    if (resident_db_check_cpf_exists(resident_db, ui->tb_cpf.input)) {
        SET_FLAG(&ui->flag, FLAG_CPF_EXISTS);
        return;
    }

    // Insert new resident
    if (resident_db_insert(
            resident_db,
            ui->tb_cpf.input,
            ui->tb_name.input,
            ui->ib_age.input,
            ui->tb_health_status.input,
            ui->tb_needs.input,
            ui->cb_medical_assistance.checked,
            ui->ddb_gender.active_option
        )
        != SQLITE_OK)
    {
        *error = ERROR_INSERT_DB;
        return;
    }

    SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
    *error = NO_ERROR;
}

/**
 * @internal
 * @brief Private function to handle the submit of a resident into the database.
 * 
 * @param ui Pointer to ui_resident struct to handle button action
 * @param resident_db Pointer to the resident database
 *
 */
static void handle_retrieve_button(struct ui_resident *ui, database *resident_db) {
    CLEAR_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);

    if (resident_db_get_by_cpf(resident_db, ui->tb_cpf.input, &ui->resident_retrieved) != SQLITE_OK) {
        SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
        return;
    }
    printf("Retrieved Person - Name: %s, Age: %d\n", ui->resident_retrieved.name, ui->resident_retrieved.age);

    SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
}

static void handle_delete_button(struct ui_resident *ui, database *resident_db) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_INPUT_CPF_EMPTY | FLAG_CPF_NOT_VALID | FLAG_CPF_NOT_FOUND | FLAG_CONFIRM_RESIDENT_DELETE
    );

    if (ui->tb_cpf.input[0] == '\0') {
        SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
        return;
    }

    if (!is_int_between_min_max(ui->tb_cpf.input, 11, 11)) {
        SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
        return;
    }

    if (!resident_db_check_cpf_exists(resident_db, ui->tb_cpf.input)) {
        SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CONFIRM_RESIDENT_DELETE);
}

/**
 * @internal
 * @brief Private function to handle the submit of a resident into the database.
 * 
 * @param ui Pointer to ui_resident struct to handle button action
 * @param resident_db Pointer to the resident database
 *
 */
static void handle_retrieve_all_button(struct ui_resident *ui, database *resident_db) {
    if (ui->table_content) {
        free(ui->table_content); // Free old data before getting new data
        ui->table_content = NULL;
    }

    int total_residents = resident_db_get_count(resident_db);
    if (total_residents == -1) {
        fprintf(stderr, "Failed to get total count.\n");
        return;
    }

    // 1024 for header + 2048 for each row
    size_t buffer_size = 1024 + 2048 * total_residents;

    ui->table_content = malloc(buffer_size);
    if (!ui->table_content) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    if (resident_db_get_all_format(resident_db, ui->table_content, buffer_size) == -1) {
        fprintf(stderr, "Failed to get formatted table.\n");
        return;
    }

    // Set the panel_content_bounds rectangle based on the width and height of the retrieved text
    if (ui->table_content) {
        Vector2 text_size = MeasureTextEx(GuiGetFont(), ui->table_content, FONT_SIZE, 0);
        ui->table_view.panel_content_bounds.width = text_size.x * 0.9;
        ui->table_view.panel_content_bounds.height = text_size.y / 0.7;
    }

    resident_db_get_all(resident_db); // also prints to stdout
    return;
}

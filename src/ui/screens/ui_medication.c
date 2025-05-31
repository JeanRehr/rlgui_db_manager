/**
 * @file ui_medication.c
 * @brief Medication screen implementation
 */
#include "ui/screens/ui_medication.h"

#include <limits.h> // For INT_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <external/raylib/raygui.h>

#include "db/medication_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_medication_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
);

static void ui_medication_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
);

static void ui_medication_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
);

static void ui_medication_update_positions(struct ui_base *base);

static void ui_medication_clear_fields(struct ui_base *base);

static void ui_medication_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_medication_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_REMOVE,
    DB_ACTION_DELETE_ENTRY, ///< We could use remove here, and just ignore the last field to be more memory eficient
    DB_ACTION_REMOVE_BY_ID,
    DB_ACTION_DELETE_ENTRY_BY_ID, ///< Same here, we could use remove by id and ignore last field
};

// Info for the database operation based on the type
struct ui_medication_db_action_info {
    enum ui_medication_db_action_type type;
    union {
        struct {
            const char *name;
            const char *form;
            const char *strength;
            int stock;
        } remove;

        struct {
            const char *name;
            const char *form;
            const char *strength;
        } delete_entry;

        struct {
            int id;
            int stock;
        } remove_by_id;

        struct {
            int id;
        } delete_entry_by_id;
    };
};

static void process_db_action_in_warning(
    struct ui_medication *ui,
    enum error_code *error,
    struct ui_medication_db_action_info *action,
    database *medication_db
);

static void draw_medication_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_medication *ui, enum app_state *state);

static void handle_insert_button(struct ui_medication *ui, enum error_code *error, database *medication_db);

static void handle_remove_button(struct ui_medication *ui);

static void handle_delete_entry_button(struct ui_medication *ui);

static void handle_remove_by_id_button(struct ui_medication *ui);

static void handle_delete_entry_by_id_button(struct ui_medication *ui);

static void handle_view_all_button(struct ui_medication *ui, database *medication_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_medication_init(struct ui_medication *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Medication");
    // Override methods
    ui->base.render = ui_medication_render;
    ui->base.handle_buttons = ui_medication_handle_buttons;
    ui->base.handle_warning_msg = ui_medication_handle_warning_msg;
    ui->base.update_positions = ui_medication_update_positions;
    ui->base.clear_fields = ui_medication_clear_fields;
    ui->base.cleanup = ui_medication_cleanup;

    // Initialize ui specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_name = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + ui->butn_back.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Medication Name:"
    );
    ui->tb_generic_name = textbox_init(
        (Rectangle) { 20, ui->tb_name.bounds.y + ui->tb_name.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Generic Name:"
    );
    ui->tb_form = textbox_init(
        (Rectangle) { 20, ui->tb_generic_name.bounds.y + ui->tb_generic_name.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Form:"
    );
    ui->tb_strength = textbox_init(
        (Rectangle) { 20, ui->tb_form.bounds.y + ui->tb_form.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Strength:"
    );
    ui->tb_unit = textbox_init(
        (Rectangle) { 20, ui->tb_strength.bounds.y + ui->tb_strength.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Unit:"
    );
    ui->ib_stock = intbox_init(
        (Rectangle) { 20, ui->tb_unit.bounds.y + ui->tb_unit.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Stock:",
        0,
        INT_MAX
    );

    ui->expirationDateText = (Rectangle) { 20,
                                           ui->ib_stock.bounds.y + ui->ib_stock.bounds.height + (FONT_SIZE * 2),
                                           MeasureText("Expiration date:", FONT_SIZE),
                                           20 };

    ui->ib_year = intbox_init(
        (Rectangle) { 20, ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2), 40, 30 },
        "Year",
        0,
        9999
    );

    ui->ib_month = intbox_init(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5,
                      ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Month",
        0,
        12
    );

    ui->ib_day = intbox_init(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5,
                      ui->expirationDateText.y + ui->expirationDateText.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Day",
        0,
        31
    );

    ui->tb_notes = textbox_init(
        (Rectangle) { 20, ui->ib_year.bounds.y + ui->ib_year.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "General notes:"
    );

    ui->ib_medication_id = intbox_init(
        (Rectangle) { 20, ui->tb_notes.bounds.y + ui->tb_notes.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "ID:",
        0,
        INT_MAX
    );

    ui->butn_insert = button_init((Rectangle) { 20, window_height - 60, 100, 30 }, "Insert");

    ui->butn_remove = button_init(
        (Rectangle) { ui->butn_insert.bounds.x + ui->butn_insert.bounds.width + 10, window_height - 60, 100, 30 },
        "Remove Quantity"
    );

    ui->butn_delete_entry = button_init(
        (Rectangle) { ui->butn_remove.bounds.x + ui->butn_remove.bounds.width + 10, window_height - 60, 100, 30 },
        "Delete Entry"
    );

    ui->butn_remove_by_id = button_init(
        (Rectangle
        ) { ui->butn_delete_entry.bounds.x + ui->butn_delete_entry.bounds.width + 10, window_height - 60, 100, 30 },
        "Remove Quantity by ID"
    );

    ui->butn_delete_entry_by_id = button_init(
        (Rectangle
        ) { ui->butn_remove_by_id.bounds.x + ui->butn_remove_by_id.bounds.width + 10, window_height - 60, 100, 30 },
        "Delete Entry by ID"
    );

    ui->butn_view_all = button_init(
        (Rectangle) { ui->butn_delete_entry_by_id.bounds.x + ui->butn_delete_entry_by_id.bounds.width + 10,
                      window_height - 60,
                      100,
                      30 },
        "View All"
    );

    ui->sp_table_view = scrollpanel_init(
        (Rectangle) { ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 10,
                      10,
                      window_width - (ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 20),
                      window_height - 100 },
        "Database view",
        (Rectangle) { 0, 0, 0, 0 }
    );

    ui->str_table_content = NULL;

    ui->flag = 0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Medication screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param medication_db Pointer to the medication database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_medication_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
) {
    struct ui_medication *ui = (struct ui_medication *)base;

    textbox_draw(&ui->tb_name);
    textbox_draw(&ui->tb_generic_name);
    textbox_draw(&ui->tb_form);
    textbox_draw(&ui->tb_strength);
    textbox_draw(&ui->tb_unit);
    intbox_draw(&ui->ib_stock);

    GuiLabel(ui->expirationDateText, "Expiration date:");

    intbox_draw(&ui->ib_year);
    GuiLabel(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width - 1,
                      ui->ib_year.bounds.y + (ui->ib_year.bounds.height / 2) - 5,
                      10,
                      10 },
        "-"
    );
    intbox_draw(&ui->ib_month);
    GuiLabel(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width - 1,
                      ui->ib_month.bounds.y + (ui->ib_month.bounds.height / 2) - 5,
                      10,
                      10 },
        "-"
    );
    intbox_draw(&ui->ib_day);

    textbox_draw(&ui->tb_notes);

    intbox_draw(&ui->ib_medication_id);

    scrollpanel_draw(&ui->sp_table_view, draw_medication_table_content, ui->str_table_content);

    ui->base.handle_buttons(&ui->base, state, error, medication_db);
    if (ui->flag != 0) {
        ui->base.handle_warning_msg(&ui->base, state, error, medication_db);
    }

    if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
    }
}

/**
 * @brief Handle button drawing and logic.
 * 
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any ui*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param medication_db Pointer to medication database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_medication_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
) {
    struct ui_medication *ui = (struct ui_medication *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_insert)) {
        handle_insert_button(ui, error, medication_db);
        return;
    }

    if (button_draw_updt(&ui->butn_remove)) {
        handle_remove_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_delete_entry)) {
        handle_delete_entry_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_remove_by_id)) {
        handle_remove_by_id_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_delete_entry_by_id)) {
        handle_delete_entry_by_id_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_view_all)) {
        handle_view_all_button(ui, medication_db);
        return;
    }
}

/**
 * @brief Manages medication-related warning/confirmation dialogs
 * 
 * @implements ui_base.handle_warning_msg
 * 
 * Shows appropriate warning messages for medication operations (e.g. removal, deletions),
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_medication*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param medication_db Pointer to medication database connection
 * 
 * @warning May trigger database operations on confirmation
 * 
 */
static void ui_medication_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *medication_db
) {
    (void)state;

    struct ui_medication *ui = (struct ui_medication *)base;

    const char *message = NULL;
    enum medication_screen_flags flag_to_clear = 0;
    struct ui_medication_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_NOTFOUND)) {
        message = "Medication not found.";
        flag_to_clear = FLAG_MEDICATION_NOTFOUND;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_STOCK_BELOW_ZERO)) {
        message = "Stock will go below zero.\nNot possible.";
        flag_to_clear = FLAG_MEDICATION_STOCK_BELOW_ZERO;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_INVALID_DATE)) {
        message = "Invalid date inserted.";
        flag_to_clear = FLAG_MEDICATION_INVALID_DATE;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_NAME_EMPTY)) {
        message = "Medication name cannot\nbe empty.";
        flag_to_clear = FLAG_MEDICATION_NAME_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_FORM_EMPTY)) {
        message = "Medication form cannot\nbe empty.";
        flag_to_clear = FLAG_MEDICATION_FORM_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_STRENGTH_EMPTY)) {
        message = "Medication strength cannot\nbe empty.";
        flag_to_clear = FLAG_MEDICATION_STRENGTH_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL)) {
        message = "Do you want to remove the\nselected quantity?";
        flag_to_clear = FLAG_MEDICATION_CONFIRM_REMOVAL;
        action.type = DB_ACTION_REMOVE;
        action.remove.name = ui->tb_name.input;
        action.remove.form = ui->tb_form.input;
        action.remove.strength = ui->tb_strength.input;
        action.remove.stock = ui->ib_stock.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION)) {
        message = "Are you sure you want to\ndelete this entry?";
        flag_to_clear = FLAG_MEDICATION_CONFIRM_DELETION;
        action.type = DB_ACTION_DELETE_ENTRY;
        action.remove.name = ui->tb_name.input;
        action.remove.form = ui->tb_form.input;
        action.remove.strength = ui->tb_strength.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL_BY_ID)) {
        message = "Do you want to remove the\nselected quantity by ID?";
        flag_to_clear = FLAG_MEDICATION_CONFIRM_REMOVAL_BY_ID;
        action.type = DB_ACTION_REMOVE_BY_ID;
        action.remove_by_id.id = ui->ib_medication_id.input;
        action.remove_by_id.stock = ui->ib_stock.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION_BY_ID)) {
        message = "Are you sure you want to\ndelete this entry by ID?";
        flag_to_clear = FLAG_MEDICATION_CONFIRM_DELETION_BY_ID;
        action.type = DB_ACTION_DELETE_ENTRY_BY_ID;
        action.delete_entry_by_id.id = ui->ib_medication_id.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR) || *error == ERROR_UPDATE_DB) {
        message = "Database error. Try Again";
        flag_to_clear = FLAG_MEDICATION_GENERIC_ERROR;
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
            process_db_action_in_warning(ui, error, &action, medication_db);
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

/**
 * @brief Updates medications UI element positions for window resizing
 * 
 * @implements ui_base.update_positions
 *
 * @param base Pointer to base UI structure (can be safely cast to ui*)
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 * 
 * @warning Should be called on window resize events
 * 
 */
static void ui_medication_update_positions(struct ui_base *base) {
    struct ui_medication *ui = (struct ui_medication *)base;

    ui->butn_insert.bounds.y = window_height - 60;
    ui->butn_remove.bounds.y = window_height - 60;
    ui->butn_delete_entry.bounds.y = window_height - 60;
    ui->butn_remove_by_id.bounds.y = window_height - 60;
    ui->butn_delete_entry_by_id.bounds.y = window_height - 60;
    ui->butn_view_all.bounds.y = window_height - 60;

    ui->sp_table_view.panel_bounds.width = window_width - (ui->tb_notes.bounds.x + ui->tb_notes.bounds.width + 20);
    ui->sp_table_view.panel_bounds.height = window_height - 100;
}

/**
 * @brief Clears all medications-related input fields
 * 
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_medication*)
 * 
 * @post All text inputs and selections are reset to defaults
 * 
 */
static void ui_medication_clear_fields(struct ui_base *base) {
    struct ui_medication *ui = (struct ui_medication *)base;

    ui->tb_name.input[0] = '\0';
    ui->tb_generic_name.input[0] = '\0';
    ui->tb_form.input[0] = '\0';
    ui->tb_strength.input[0] = '\0';
    ui->tb_unit.input[0] = '\0';
    ui->ib_stock.input = 0;
    ui->ib_year.input = 0;
    ui->ib_month.input = 0;
    ui->ib_day.input = 0;
    ui->tb_notes.input[0] = '\0';
    ui->ib_medication_id.input = 0;
}

/**
 * @brief Cleans up medications screen resources
 * 
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_medication*)
 * 
 * @warning Frees any allocated buffers/memory
 * 
 */
static void ui_medication_cleanup(struct ui_base *base) {
    struct ui_medication *ui = (struct ui_medication *)base;

    if (ui->str_table_content) {
        free(ui->str_table_content);
        ui->str_table_content = NULL;
    }
}

/** @} */

/* ======================= INTERNAL HELPERS ======================= */

/**
 * @internal
 * @brief Processes database actions triggered by warning messages
 * 
 * Handles medications removal, deletion and other DB operations that may be requested
 * through warning message dialogs.
 * 
 * @param ui UI context
 * @param error Error code to set if operation fails
 * @param action Database action to perform with parameters
 * @param medication_db Database connection
 *
 */
static void process_db_action_in_warning(
    struct ui_medication *ui,
    enum error_code *error,
    struct ui_medication_db_action_info *action,
    database *medication_db
) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_MEDICATION_NOTFOUND | FLAG_MEDICATION_STOCK_BELOW_ZERO | FLAG_MEDICATION_GENERIC_ERROR
            | FLAG_MEDICATION_OPERATION_DONE
    );
    switch (action->type) {
    case DB_ACTION_REMOVE: {
        int rc = medication_db_remove(
            medication_db,
            action->remove.name,
            action->remove.form,
            action->remove.strength,
            action->remove.stock
        );

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_NOTFOUND);
            fprintf(stderr, "Medication entry not found\n");
            return;
        } else if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_STOCK_BELOW_ZERO);
            fprintf(stderr, "Cannot remove medication, stock will go below 0.\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            *error = ERROR_DELETE_DB;
            return;
        }

        SET_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
        *error = NO_ERROR;
        break;
    }
    case DB_ACTION_DELETE_ENTRY: {
        int rc = medication_db_delete_entry(
            medication_db,
            action->delete_entry.name,
            action->delete_entry.form,
            action->delete_entry.strength
        );

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_NOTFOUND);
            fprintf(stderr, "Medication entry not found\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            return;
        }

        SET_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
        break;
    }
    case DB_ACTION_REMOVE_BY_ID: {
        int rc = medication_db_remove_by_id(medication_db, action->remove_by_id.id, action->remove_by_id.stock);

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_NOTFOUND);
            fprintf(stderr, "Medication entry not found\n");
            return;
        } else if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_STOCK_BELOW_ZERO);
            fprintf(stderr, "Cannot remove medication, stock will go below 0.\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            *error = ERROR_DELETE_DB;
            return;
        }

        SET_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
        *error = NO_ERROR;
        break;
    }
    case DB_ACTION_DELETE_ENTRY_BY_ID: {
        int rc = medication_db_delete_entry_by_id(medication_db, action->delete_entry_by_id.id);

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_NOTFOUND);
            fprintf(stderr, "Medication entry not found\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            return;
        }

        SET_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
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
static void draw_medication_table_content(Rectangle bounds, char *data) {
    GuiLabel(bounds, data ? data : "No data");
}

static void handle_back_button(struct ui_medication *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);
    *state = STATE_MAIN_MENU;
}

static void handle_insert_button(struct ui_medication *ui, enum error_code *error, database *medication_db) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_MEDICATION_NAME_EMPTY | FLAG_MEDICATION_FORM_EMPTY | FLAG_MEDICATION_STRENGTH_EMPTY
            | FLAG_MEDICATION_INVALID_DATE | FLAG_MEDICATION_GENERIC_ERROR | FLAG_MEDICATION_OPERATION_DONE
    );

    // Validate inputs
    if (ui->tb_name.input[0] == '\0') {
        fprintf(stderr, "Name cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_MEDICATION_NAME_EMPTY);
        return;
    }

    if (ui->tb_form.input[0] == '\0') {
        fprintf(stderr, "Form cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_MEDICATION_FORM_EMPTY);
        return;
    }

    if (ui->tb_strength.input[0] == '\0') {
        fprintf(stderr, "Strength cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_MEDICATION_STRENGTH_EMPTY);
        return;
    }

    bool is_valid_date = validate_date(ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);

    if (!is_valid_date) {
        fprintf(
            stderr,
            "Date not valid, year: %d, month: %d, day: %d\n",
            ui->ib_year.input,
            ui->ib_month.input,
            ui->ib_day.input
        );
        SET_FLAG(&ui->flag, FLAG_MEDICATION_INVALID_DATE);
        return;
    }

    char date_string[DATE_LEN] = { 0 };

    snprintf(
        date_string,
        sizeof(date_string),
        "%04d-%02d-%02d",
        ui->ib_year.input,
        ui->ib_month.input,
        ui->ib_day.input
    );

    if (medication_db_upsert(
            medication_db,
            ui->tb_name.input,
            ui->tb_generic_name.input,
            ui->tb_form.input,
            ui->tb_strength.input,
            ui->tb_unit.input,
            ui->ib_stock.input,
            date_string,
            ui->tb_notes.input
        )
        != SQLITE_OK)
    {
        fprintf(stderr, "Database operation error.\n");
        SET_FLAG(&ui->flag, FLAG_MEDICATION_GENERIC_ERROR);
        *error = ERROR_INSERT_DB;
        return;
    }

    SET_FLAG(&ui->flag, FLAG_MEDICATION_OPERATION_DONE);
    *error = NO_ERROR;
    return;
}

static void handle_remove_button(struct ui_medication *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL);

    SET_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL);
}

static void handle_delete_entry_button(struct ui_medication *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION);

    SET_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION);
}

static void handle_remove_by_id_button(struct ui_medication *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL_BY_ID);

    SET_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_REMOVAL_BY_ID);
}

static void handle_delete_entry_by_id_button(struct ui_medication *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION_BY_ID);

    SET_FLAG(&ui->flag, FLAG_MEDICATION_CONFIRM_DELETION_BY_ID);
}

static void handle_view_all_button(struct ui_medication *ui, database *medication_db) {
    if (ui->str_table_content) {
        free(ui->str_table_content); // Free old data before getting new data
        ui->str_table_content = NULL;
    }

    int total_medication = medication_db_get_count(medication_db);
    if (total_medication == -1) {
        fprintf(stderr, "Failed to get total count.\n");
        return;
    }

    // 1024 for header + 2048 for each row
    size_t buffer_size = 1024 + 2048 * total_medication;

    ui->str_table_content = malloc(buffer_size);
    if (!ui->str_table_content) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    if (medication_db_get_all_format(medication_db, ui->str_table_content, buffer_size) == -1) {
        fprintf(stderr, "Failed to get formatted table.\n");
        free(ui->str_table_content);
        ui->str_table_content = NULL;
        return;
    }

    // Set the panel_content_bounds rectangle based on the width and height of the retrieved text
    if (ui->str_table_content) {
        Vector2 text_size = MeasureTextEx(GuiGetFont(), ui->str_table_content, FONT_SIZE, 0);
        ui->sp_table_view.panel_content_bounds.width = text_size.x * 0.9;
        ui->sp_table_view.panel_content_bounds.height = text_size.y / 0.7;
    }

    medication_db_get_all(medication_db);
    return;
}

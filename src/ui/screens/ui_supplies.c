/**
 * @file ui_supplies.c
 * @brief Supplies screen implementation
 */
#include "ui/screens/ui_supplies.h"

#include <limits.h> // For INT_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <external/raylib/raygui.h>

#include "db/supplies_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_supplies_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
);

static void ui_supplies_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
);

static void ui_supplies_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
);

static void ui_supplies_update_positions(struct ui_base *base);

static void ui_supplies_clear_fields(struct ui_base *base);

static void ui_supplies_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_supplies_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_REMOVE,
    DB_ACTION_DELETE_ENTRY, ///< We could use remove here, and just ignore the last field to be more memory eficient
    DB_ACTION_REMOVE_BY_ID,
    DB_ACTION_DELETE_ENTRY_BY_ID, ///< Same here, we could use remove by id and ignore last field
};

// Info for the database operation based on the type
struct ui_supplies_db_action_info {
    enum ui_supplies_db_action_type type;
    union {
        struct {
            const char *name;
            const char *category;
            const char *size;
            int stock;
        } remove;

        struct {
            const char *name;
            const char *category;
            const char *size;
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
    struct ui_supplies *ui,
    enum error_code *error,
    struct ui_supplies_db_action_info *action,
    database *supplies_db
);

static void draw_supplies_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_supplies *ui, enum app_state *state);

static void handle_insert_button(struct ui_supplies *ui, enum error_code *error, database *supplies_db);

static void handle_remove_button(struct ui_supplies *ui);

static void handle_delete_entry_button(struct ui_supplies *ui);

static void handle_remove_by_id_button(struct ui_supplies *ui);

static void handle_delete_entry_by_id_button(struct ui_supplies *ui);

static void handle_view_all_button(struct ui_supplies *ui, database *supplies_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_supplies_init(struct ui_supplies *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "Supplies");
    // Override methods
    ui->base.render = ui_supplies_render;
    ui->base.handle_buttons = ui_supplies_handle_buttons;
    ui->base.handle_warning_msg = ui_supplies_handle_warning_msg;
    ui->base.update_positions = ui_supplies_update_positions;
    ui->base.clear_fields = ui_supplies_clear_fields;
    ui->base.cleanup = ui_supplies_cleanup;

    // Initialize ui specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_name = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + ui->butn_back.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Supply Name:"
    );

    ui->tb_category = textbox_init(
        (Rectangle) { 20, ui->tb_name.bounds.y + ui->tb_name.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Category:"
    );

    ui->tb_size = textbox_init(
        (Rectangle) { 20, ui->tb_category.bounds.y + ui->tb_category.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Size:"
    );

    ui->tb_unit = textbox_init(
        (Rectangle) { 20, ui->tb_size.bounds.y + ui->tb_size.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Unit:"
    );

    ui->ib_stock = intbox_init(
        (Rectangle) { 20, ui->tb_unit.bounds.y + ui->tb_unit.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Quantity:",
        0,
        INT_MAX
    );

    ui->tb_notes = textbox_init(
        (Rectangle) { 20, ui->ib_stock.bounds.y + ui->ib_stock.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Notes:"
    );

    ui->ib_supply_id = intbox_init(
        (Rectangle) { 20, ui->tb_notes.bounds.y + ui->tb_notes.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Entry ID:",
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
        (
            Rectangle
        ) { ui->butn_delete_entry.bounds.x + ui->butn_delete_entry.bounds.width + 10, window_height - 60, 100, 30 },
        "Remove Quantity by ID"
    );

    ui->butn_delete_entry_by_id = button_init(
        (
            Rectangle
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
 * @brief Supplies screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param supplies_db Pointer to the supplies database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_supplies_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
) {
    struct ui_supplies *ui = (struct ui_supplies *)base;

    textbox_draw(&ui->tb_name);
    textbox_draw(&ui->tb_category);
    textbox_draw(&ui->tb_size);
    textbox_draw(&ui->tb_unit);
    intbox_draw(&ui->ib_stock);
    textbox_draw(&ui->tb_notes);
    intbox_draw(&ui->ib_supply_id);

    scrollpanel_draw(&ui->sp_table_view, draw_supplies_table_content, ui->str_table_content);

    ui->base.handle_buttons(&ui->base, state, error, supplies_db);
    // Start show warning/error boxes (only if there is a flag set)
    if (ui->flag != 0) {
        ui->base.handle_warning_msg(&ui->base, state, error, supplies_db);
    }

    if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
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
 * @param supplies_db Pointer to supplies database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_supplies_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
) {
    (void)error;
    (void)supplies_db;

    struct ui_supplies *ui = (struct ui_supplies *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_insert)) {
        handle_insert_button(ui, error, supplies_db);
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
        handle_view_all_button(ui, supplies_db);
        return;
    }
}

/**
 * @brief Manages supplies-related warning/confirmation dialogs
 * 
 * @implements ui_base.handle_warning_msg
 * 
 * Shows appropriate warning messages for supplies operations (e.g. removal, deletions),
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_supplies*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param supplies_db Pointer to supplies database connection
 * 
 * @warning May trigger database operations on confirmation
 * 
 */
static void ui_supplies_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *supplies_db
) {
    (void)state;

    struct ui_supplies *ui = (struct ui_supplies *)base;

    const char *message = NULL;
    enum supplies_screen_flags flag_to_clear = 0;
    struct ui_supplies_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_NOTFOUND)) {
        message = "Supply not found.";
        flag_to_clear = FLAG_SUPPLIES_NOTFOUND;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_STOCK_BELOW_ZERO)) {
        message = "Stock will go below zero.\nNot possible.";
        flag_to_clear = FLAG_SUPPLIES_STOCK_BELOW_ZERO;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_NAME_EMPTY)) {
        message = "Supply name cannot\nbe empty.";
        flag_to_clear = FLAG_SUPPLIES_NAME_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_CATEGORY_EMPTY)) {
        message = "Supply category cannot\nbe empty.";
        flag_to_clear = FLAG_SUPPLIES_CATEGORY_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_SIZE_EMPTY)) {
        message = "Supply size cannot\nbe empty.";
        flag_to_clear = FLAG_SUPPLIES_SIZE_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL)) {
        message = "Do you want to remove the\nselected quantity?";
        flag_to_clear = FLAG_SUPPLIES_CONFIRM_REMOVAL;
        action.type = DB_ACTION_REMOVE;
        action.remove.name = ui->tb_name.input;
        action.remove.category = ui->tb_category.input;
        action.remove.size = ui->tb_size.input;
        action.remove.stock = ui->ib_stock.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION)) {
        message = "Are you sure you want to\ndelete this entry?";
        flag_to_clear = FLAG_SUPPLIES_CONFIRM_DELETION;
        action.type = DB_ACTION_DELETE_ENTRY;
        action.remove.name = ui->tb_name.input;
        action.remove.category = ui->tb_category.input;
        action.remove.size = ui->tb_size.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL_BY_ID)) {
        message = "Do you want to remove the\nselected quantity by ID?";
        flag_to_clear = FLAG_SUPPLIES_CONFIRM_REMOVAL_BY_ID;
        action.type = DB_ACTION_REMOVE_BY_ID;
        action.remove_by_id.id = ui->ib_supply_id.input;
        action.remove_by_id.stock = ui->ib_stock.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION_BY_ID)) {
        message = "Are you sure you want to\ndelete this entry by ID?";
        flag_to_clear = FLAG_SUPPLIES_CONFIRM_DELETION_BY_ID;
        action.type = DB_ACTION_DELETE_ENTRY_BY_ID;
        action.delete_entry_by_id.id = ui->ib_supply_id.input;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR) || *error == ERROR_UPDATE_DB) {
        message = "Database error. Try Again";
        flag_to_clear = FLAG_SUPPLIES_GENERIC_ERROR;
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
            process_db_action_in_warning(ui, error, &action, supplies_db);
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

/**
 * @brief Updates supplies UI element positions for window resizing
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
static void ui_supplies_update_positions(struct ui_base *base) {
    struct ui_supplies *ui = (struct ui_supplies *)base;

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
 * @brief Clears all supplies-related input fields
 * 
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_supplies*)
 * 
 * @post All text inputs and selections are reset to defaults
 * 
 */
static void ui_supplies_clear_fields(struct ui_base *base) {
    struct ui_supplies *ui = (struct ui_supplies *)base;

    ui->tb_name.input[0] = '\0';
    ui->tb_category.input[0] = '\0';
    ui->tb_size.input[0] = '\0';
    ui->tb_unit.input[0] = '\0';
    ui->ib_stock.input = 0;
    ui->tb_notes.input[0] = '\0';
    ui->ib_supply_id.input = 0;
}

/**
 * @brief Cleans up supplies screen resources
 * 
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_supplies*)
 * 
 * @warning Frees any allocated buffers/memory
 * 
 */
static void ui_supplies_cleanup(struct ui_base *base) {
    struct ui_supplies *ui = (struct ui_supplies *)base;

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
 * Handles supplies removal, deletion and other DB operations that may be requested
 * through warning message dialogs.
 * 
 * @param ui UI context
 * @param error Error code to set if operation fails
 * @param action Database action to perform with parameters
 * @param supplies_db Database connection
 *
 */
static void process_db_action_in_warning(
    struct ui_supplies *ui,
    enum error_code *error,
    struct ui_supplies_db_action_info *action,
    database *supplies_db
) {
    CLEAR_FLAG(
        &ui->flag,
        FLAG_SUPPLIES_NOTFOUND | FLAG_SUPPLIES_STOCK_BELOW_ZERO | FLAG_SUPPLIES_GENERIC_ERROR
            | FLAG_SUPPLIES_OPERATION_DONE
    );
    switch (action->type) {
    case DB_ACTION_REMOVE: {
        int rc = supplies_db_remove(
            supplies_db,
            action->remove.name,
            action->remove.category,
            action->remove.size,
            action->remove.stock
        );

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_NOTFOUND);
            fprintf(stderr, "Supplies entry not found\n");
            return;
        } else if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_STOCK_BELOW_ZERO);
            fprintf(stderr, "Cannot remove supply, stock will go below 0.\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            *error = ERROR_DELETE_DB;
            return;
        }

        SET_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
        *error = NO_ERROR;
        break;
    }
    case DB_ACTION_DELETE_ENTRY: {
        int rc = supplies_db_delete_entry(
            supplies_db,
            action->delete_entry.name,
            action->delete_entry.category,
            action->delete_entry.size
        );

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_NOTFOUND);
            fprintf(stderr, "Supplies entry not found\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            return;
        }

        SET_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
        break;
    }
    case DB_ACTION_REMOVE_BY_ID: {
        int rc = supplies_db_remove_by_id(supplies_db, action->remove_by_id.id, action->remove_by_id.stock);

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_NOTFOUND);
            fprintf(stderr, "Supplies entry not found\n");
            return;
        } else if (rc == SQLITE_CONSTRAINT) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_STOCK_BELOW_ZERO);
            fprintf(stderr, "Cannot remove supply, stock will go below 0.\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            *error = ERROR_DELETE_DB;
            return;
        }

        SET_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
        *error = NO_ERROR;
        break;
    }
    case DB_ACTION_DELETE_ENTRY_BY_ID: {
        int rc = supplies_db_delete_entry_by_id(supplies_db, action->delete_entry_by_id.id);

        if (rc == SQLITE_NOTFOUND) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_NOTFOUND);
            fprintf(stderr, "Supplies entry not found\n");
            return;
        } else if (rc != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR);
            fprintf(stderr, "An error occurred during database operation.\n");
            return;
        }

        SET_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
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
static void draw_supplies_table_content(Rectangle bounds, char *data) {
    GuiLabel(bounds, data ? data : "No data");
}

static void handle_back_button(struct ui_supplies *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);
    *state = STATE_MAIN_MENU;
}

static void handle_insert_button(struct ui_supplies *ui, enum error_code *error, database *supplies_db) {
    CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);

    // Validate inputs
    if (ui->tb_name.input[0] == '\0') {
        fprintf(stderr, "Name cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_SUPPLIES_NAME_EMPTY);
        return;
    }

    if (ui->tb_category.input[0] == '\0') {
        fprintf(stderr, "Category cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_SUPPLIES_CATEGORY_EMPTY);
        return;
    }

    if (ui->tb_size.input[0] == '\0') {
        fprintf(stderr, "Size cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_SUPPLIES_SIZE_EMPTY);
        return;
    }

    if (supplies_db_upsert(
        supplies_db,
        ui->tb_name.input,
        ui->tb_category.input,
        ui->tb_size.input,
        ui->tb_unit.input,
        ui->ib_stock.input,
        ui->tb_notes.input
    ) != SQLITE_OK) {
        SET_FLAG(&ui->flag, FLAG_SUPPLIES_GENERIC_ERROR);
        fprintf(stderr, "Database error.");
        return;
    }

    SET_FLAG(&ui->flag, FLAG_SUPPLIES_OPERATION_DONE);
    *error = NO_ERROR;
    return;
}

static void handle_remove_button(struct ui_supplies *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL);

    SET_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL);
}

static void handle_delete_entry_button(struct ui_supplies *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION);

    SET_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION);
}

static void handle_remove_by_id_button(struct ui_supplies *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL_BY_ID);

    SET_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_REMOVAL_BY_ID);
}

static void handle_delete_entry_by_id_button(struct ui_supplies *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION_BY_ID);

    SET_FLAG(&ui->flag, FLAG_SUPPLIES_CONFIRM_DELETION_BY_ID);
}

static void handle_view_all_button(struct ui_supplies *ui, database *supplies_db) {
    if (ui->str_table_content) {
        free(ui->str_table_content); // Free old data before getting new data
        ui->str_table_content = NULL;
    }

    
    int total_supplies = supplies_db_get_count(supplies_db);
    if (total_supplies == -1) {
        fprintf(stderr, "Failed to get total count.\n");
        return;
    }

    // 406 for header + 1450 for each row
    size_t buffer_size = 512 + 2048 * total_supplies;

    ui->str_table_content = malloc(buffer_size);
    if (!ui->str_table_content) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    if (supplies_db_get_all_format(supplies_db, ui->str_table_content, buffer_size) == -1) {
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

    supplies_db_get_all(supplies_db);
    return;
}

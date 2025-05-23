/**
 * @file ui_food.c
 * @brief Ui food screen implementation
 */
#include "ui/ui_food.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <external/raylib/raygui.h>

#include "db/foodbatch_db.h"
#include "globals.h"
#include "utilsfn.h"

/* Forward declarations */

static void ui_food_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *foodbatch_db);

static void ui_food_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *foodbatch_db
);

static void ui_food_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *foodbatch_db
);

static void ui_food_update_positions(struct ui_base *base);

static void ui_food_clear_fields(struct ui_base *base);

static void ui_food_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_food_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_UPDATE,
    DB_ACTION_DELETE,
};

// Info for the database operation based on the type
struct ui_food_db_action_info {
    enum ui_food_db_action_type type;
    union {
        struct {
            int batch_id;
            const char *name;
            int quantity;
            bool is_perishable;
            const char *date_string;
            float daily_consumption_rate;
        } update;

        struct {
            int batch_id;
        } delete;
    };
};

static void process_db_action_in_warning(
    struct ui_food *ui,
    enum error_code *error,
    struct ui_food_db_action_info *action,
    database *foodbatch_db
);

static void draw_foodbatch_info_panel(struct ui_food *ui);

static void draw_foodbatch_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_food *ui, enum app_state *state);

static void handle_submit_button(struct ui_food *ui, enum error_code *error, database *foodbatch_db);

static void handle_retrieve_button(struct ui_food *ui, database *foodbatch_db);

static void handle_delete_button(struct ui_food *ui, database *foodbatch_db);

static void handle_retrieve_all_button(struct ui_food *ui, database *foodbatch_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_food_init(struct ui_food *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, "ui_food.c");

    // Override methods
    ui->base.render = ui_food_render;
    ui->base.handle_buttons = ui_food_handle_buttons;
    ui->base.handle_warning_msg = ui_food_handle_warning_msg;
    ui->base.update_positions = ui_food_update_positions;
    ui->base.clear_fields = ui_food_clear_fields;
    ui->base.cleanup = ui_food_cleanup;

    // UI Resident specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->ib_batch_id = intbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 130, 30 },
        "Batch ID:",
        0,
        99999999
    );

    ui->tb_name = textbox_init(
        (Rectangle) { 20, ui->ib_batch_id.bounds.y + (ui->ib_batch_id.bounds.height * 2), 300, 30 },
        "Food Name:"
    );

    ui->ib_quantity = intbox_init(
        (Rectangle) { 20, ui->tb_name.bounds.y + (ui->tb_name.bounds.height * 2), 125, 30 },
        "Quantity:",
        0,
        INT_MAX
    );

    ui->cb_is_perishable = checkbox_init(
        (Rectangle) { 20, ui->ib_quantity.bounds.y + (ui->ib_quantity.bounds.height * 2), 20, 20 },
        "Is Perishable?"
    );

    ui->expirationDateText = (Rectangle) { 20,
                                           ui->cb_is_perishable.bounds.y + (ui->cb_is_perishable.bounds.height * 2),
                                           MeasureText("Expiration date:", FONT_SIZE),
                                           20 };

    ui->ib_year = intbox_init(
        (Rectangle) { 20, ui->expirationDateText.y + (ui->expirationDateText.height * 2), 40, 30 },
        "Year",
        0,
        9999
    );

    ui->ib_month = intbox_init(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5,
                      ui->expirationDateText.y + (ui->expirationDateText.height * 2),
                      35,
                      30 },
        "Month",
        0,
        12
    );

    ui->ib_day = intbox_init(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5,
                      ui->expirationDateText.y + (ui->expirationDateText.height * 2),
                      35,
                      30 },
        "Day",
        0,
        31
    );

    ui->fb_daily_consumption_rate = floatbox_init(
        (Rectangle) { 20, ui->ib_year.bounds.y + (ui->ib_year.bounds.height * 2), 250, 30 },
        "Avg daily consumption rate per resident?"
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

    memset(&ui->foodbatch_retrieved, 0, sizeof(struct foodbatch));

    // Only set the bounds of the panel, draw everything inside based on it on the draw register resident screen function
    ui->panel_bounds = (Rectangle) { ui->tb_name.bounds.x + ui->tb_name.bounds.width + 10, 10, 300, 250 };

    ui->sp_table_view = scrollpanel_init(
        (Rectangle) { ui->panel_bounds.x + ui->panel_bounds.width + 10,
                      10,
                      window_width - (ui->panel_bounds.x + ui->panel_bounds.width + 20),
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
 * @brief Implementation of ui_base function pointers for food management
 * @{
 */

/**
 * @brief Renders food management screen and handles interactions
 * 
 * @implements ui_base.render
 * 
 * @param base Must cast to ui_food*
 * @param state Modified on screen transition
 * @param error Set on database failures
 * @param foodbatch_db Food database connection
 * 
 * @warning Immediate-mode rendering (draws and handles input in one pass)
 * 
 */
static void ui_food_render(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *foodbatch_db
) {
    struct ui_food *ui = (struct ui_food *)base;

    // Start draw UI elements

    intbox_draw(&ui->ib_batch_id);
    textbox_draw(&ui->tb_name);
    intbox_draw(&ui->ib_quantity);

    checkbox_draw(&ui->cb_is_perishable);

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

    if (ui->cb_is_perishable.checked == false
        && (ui->ib_year.input != 1 || ui->ib_month.input != 1 || ui->ib_day.input != 1))
    {
        // Setting a default value for inserting into the database
        ui->ib_year.input = 1;
        ui->ib_month.input = 1;
        ui->ib_day.input = 1;
    } else if (ui->cb_is_perishable.checked == true
               && (ui->ib_year.input == 1 && ui->ib_month.input == 1 && ui->ib_day.input == 1))
    {
        // This is to set values to 0 when the check goes from false to true
        ui->ib_year.input = 0;
        ui->ib_month.input = 0;
        ui->ib_day.input = 0;
    }

    floatbox_draw(&ui->fb_daily_consumption_rate);

    // Start Info Panel
    draw_foodbatch_info_panel(ui);

    // Draw database content
    scrollpanel_draw(&ui->sp_table_view, draw_foodbatch_table_content, ui->str_table_content);

    // End draw UI elements

    // Start button actions
    ui->base.handle_buttons(&ui->base, state, error, foodbatch_db);

    // Start show warning/error boxes
    ui->base.handle_warning_msg(&ui->base, state, error, foodbatch_db);

    // Clear the text buffer only after a successful operation
    if (IS_FLAG_SET(&ui->flag, FLAG_FOOD_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
    }
}

/**
 * @brief Handle food screen button drawing and logic
 * 
 * @implements ui_base.handle_buttons
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_food*)
 * @param state Pointer to application state (modified on button actions)
 * @param error Pointer to error tracking variable
 * @param foodbatch_db Pointer to food database connection
 * 
 * @warning Should be called through the base interface
 * 
 */
static void ui_food_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *foodbatch_db
) {
    struct ui_food *ui = (struct ui_food *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_submit)) {
        handle_submit_button(ui, error, foodbatch_db);
        return;
    }

    if (button_draw_updt(&ui->butn_retrieve)) {
        handle_retrieve_button(ui, foodbatch_db);
        return;
    }

    if (button_draw_updt(&ui->butn_delete)) {
        handle_delete_button(ui, foodbatch_db);
        return;
    }

    if (button_draw_updt(&ui->butn_retrieve_all)) {
        handle_retrieve_all_button(ui, foodbatch_db);
        return;
    }

    return;
}

/**
 * @brief Manages food-related warning/confirmation dialogs
 * 
 * @implements ui_base.handle_warning_msg
 * 
 * Shows appropriate warning messages for food operations (e.g., deletions),
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_food*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param foodbatch_db Pointer to food database connection
 * 
 * @warning May trigger database operations on confirmation
 * 
 */
static void ui_food_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *foodbatch_db
) {
    (void)state;

    struct ui_food *ui = (struct ui_food *)base;

    const char *message = NULL;
    enum food_screen_flags flag_to_clear = 0;
    struct ui_food_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    // Warnings
    if (IS_FLAG_SET(&ui->flag, FLAG_BATCHID_NOT_FOUND)) {
        message = "Batch ID not found.";
        flag_to_clear = FLAG_BATCHID_NOT_FOUND;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_INVALID_FOOD_DATE)) {
        message = "Date inserted is not valid.";
        flag_to_clear = FLAG_INVALID_FOOD_DATE;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_BATCHID_EXISTS)) {
        message = "Batch ID already exists. Update?";
        flag_to_clear = FLAG_BATCHID_EXISTS;
        action.type = DB_ACTION_UPDATE;
        action.update.batch_id = ui->ib_batch_id.input;
        action.update.name = ui->tb_name.input;
        action.update.quantity = ui->ib_quantity.input;
        action.update.is_perishable = ui->cb_is_perishable.checked;
        // Don't need to validate date here, as it's impossible to be wrong at this stage
        char date_string[11] = { 0 }; // YYYY-MM-DD + null terminator
        snprintf(
            date_string,
            sizeof(date_string),
            "%04d-%02d-%02d",
            ui->ib_year.input,
            ui->ib_month.input,
            ui->ib_day.input
        );
        action.update.date_string = date_string;
        action.update.daily_consumption_rate = ui->fb_daily_consumption_rate.value;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_CONFIRM_FOOD_DELETE)) {
        message = "Are you sure you want to delete\nthis foodbatch?";
        flag_to_clear = FLAG_CONFIRM_FOOD_DELETE;
        action.type = DB_ACTION_DELETE;
        action.delete.batch_id = ui->ib_batch_id.input;
    } else if (*error == ERROR_INSERT_DB || *error == ERROR_UPDATE_DB) {
        message = "Database error. Try again.";
        *error = NO_ERROR;
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
            process_db_action_in_warning(ui, error, &action, foodbatch_db);
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

/**
 * @brief Updates food UI element positions for window resizing
 * 
 * @implements ui_base.update_positions
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_food*)
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 * 
 * @warning Should be called on window resize events
 * 
 */
static void ui_food_update_positions(struct ui_base *base) {
    struct ui_food *ui = (struct ui_food *)base;

    ui->butn_submit.bounds.y = window_height - 60;
    ui->butn_retrieve.bounds.y = ui->butn_submit.bounds.y;
    ui->butn_delete.bounds.y = ui->butn_submit.bounds.y;
    ui->butn_retrieve_all.bounds.y = ui->butn_submit.bounds.y;
    ui->sp_table_view.panel_bounds.width =
        window_width - (ui->panel_bounds.x + ui->panel_bounds.width + 20);
    ui->sp_table_view.panel_bounds.height = window_height - 100;
}

/**
 * @brief Clears all food-related input fields
 * 
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_food*)
 * 
 * @post All text inputs and selections are reset to defaults
 * 
 */
static void ui_food_clear_fields(struct ui_base *base) {
    struct ui_food *ui = (struct ui_food *)base;

    ui->ib_batch_id.input = 0;
    ui->tb_name.input[0] = '\0';
    ui->ib_quantity.input = 0;
    ui->cb_is_perishable.checked = false;
    ui->ib_year.input = 0;
    ui->ib_month.input = 0;
    ui->ib_day.input = 0;
    ui->fb_daily_consumption_rate.value = 0;
    ui->fb_daily_consumption_rate.text_input[0] = '\0';
}

/**
 * @brief Cleans up food screen resources
 * 
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_food*)
 * 
 * @warning Frees any allocated buffers/memory
 * 
 */
static void ui_food_cleanup(struct ui_base *base) {
    struct ui_food *ui = (struct ui_food *)base;

    if (ui->str_table_content) {
        free(ui->str_table_content);
        ui->str_table_content = NULL; // Prevent double-free
    }
}
/** @} */

/* ======================= INTERNAL HELPERS ======================= */

static void draw_foodbatch_info_panel(struct ui_food *ui) {
    GuiPanel(ui->panel_bounds, TextFormat("Batch ID retrieved: %d", ui->foodbatch_retrieved.batch_id));

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20 },
        TextFormat("Name: %s", ui->foodbatch_retrieved.name)
    );
    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20 },
        TextFormat("Quantity: %d", ui->foodbatch_retrieved.quantity)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20 },
        TextFormat("Expiration date: %s", ui->foodbatch_retrieved.expiration_date)
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20 },
        TextFormat("Is Perishable: %s", ui->foodbatch_retrieved.is_perishable == true ? "True" : "False")
    );

    GuiLabel(
        (Rectangle) { ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20 },
        TextFormat("Daily Consumption Rate: %.2f", ui->foodbatch_retrieved.daily_consumption_rate)
    );
}

/**
 * @internal
 * @brief Draws the table content of the database
 * 
 * @note This is a callback to be used in the scrollpanel_draw
 * 
 */
static void draw_foodbatch_table_content(Rectangle bounds, char *data) {
    GuiLabel(bounds, data ? data : "No data");
}

static void handle_back_button(struct ui_food *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);

    *state = STATE_MAIN_MENU;
    return;
}

static void handle_submit_button(struct ui_food *ui, enum error_code *error, database *foodbatch_db) {
    // Clear flags
    CLEAR_FLAG(&ui->flag, FLAG_BATCHID_EXISTS | FLAG_INVALID_FOOD_DATE);

    // Validate inputs
    bool is_valid_date = validate_date(ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);

    if (!is_valid_date) {
        printf(
            "Date not valid, year: %d, month: %d, day: %d\n",
            ui->ib_year.input,
            ui->ib_month.input,
            ui->ib_day.input
        );
        SET_FLAG(&ui->flag, FLAG_INVALID_FOOD_DATE);
        return;
    }

    if (foodbatch_db_check_batchid_exists(foodbatch_db, ui->ib_batch_id.input)) {
        SET_FLAG(&ui->flag, FLAG_BATCHID_EXISTS);
        printf("Food batch ID exists: %d\n", ui->ib_batch_id.input);
        return;
    }

    char date_string[11] = { 0 }; // YYYY-MM-DD + null terminator

    snprintf(
        date_string,
        sizeof(date_string),
        "%04d-%02d-%02d",
        ui->ib_year.input,
        ui->ib_month.input,
        ui->ib_day.input
    );

    if (foodbatch_db_insert(
            foodbatch_db,
            ui->ib_batch_id.input,
            ui->tb_name.input,
            ui->ib_quantity.input,
            ui->cb_is_perishable.checked,
            date_string,
            ui->fb_daily_consumption_rate.value
        )
        != SQLITE_OK)
    {
        *error = ERROR_INSERT_DB;
        fprintf(stderr, "Error submitting to database.\n");
        return;
    }

    SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
    *error = NO_ERROR;
}

static void handle_retrieve_button(struct ui_food *ui, database *foodbatch_db) {
    CLEAR_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);

    if (foodbatch_db_get_by_batchid(foodbatch_db, ui->ib_batch_id.input, &ui->foodbatch_retrieved) != SQLITE_OK) {
        SET_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);
    }

    printf(
        "Retrieved Food Batch - Name: %s, Qauntity: %d, Is Perishable: %d, Expiration Date: %s, Daily Consumption Rate: %f\n",
        ui->foodbatch_retrieved.name,
        ui->foodbatch_retrieved.quantity,
        ui->foodbatch_retrieved.is_perishable,
        ui->foodbatch_retrieved.expiration_date,
        ui->foodbatch_retrieved.daily_consumption_rate
    );

    SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
}

static void handle_delete_button(struct ui_food *ui, database *foodbatch_db) {
    CLEAR_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND | FLAG_CONFIRM_FOOD_DELETE);

    if (!foodbatch_db_check_batchid_exists(foodbatch_db, ui->ib_batch_id.input)) {
        SET_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);
        return;
    }

    SET_FLAG(&ui->flag, FLAG_CONFIRM_FOOD_DELETE);
}

static void handle_retrieve_all_button(struct ui_food *ui, database *foodbatch_db) {
    if (ui->str_table_content) {
        free(ui->str_table_content); // Free old data before getting new data
        ui->str_table_content = NULL;
    }

    int total_foodbatch = foodbatch_db_get_count(foodbatch_db);
    if (total_foodbatch == -1) {
        fprintf(stderr, "Failed to get total count.\n");
        return;
    }

    // 512 for header + 512 for each row as documented on foodbatch_db_get_all_format
    size_t buffer_size = 512 + 512 * total_foodbatch;

    ui->str_table_content = malloc(buffer_size);
    if (!ui->str_table_content) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    if (foodbatch_db_get_all_format(foodbatch_db, ui->str_table_content, buffer_size) == -1) {
        fprintf(stderr, "Failed to get formatted table.\n");
        return;
    }

    // Set the panel_content_bounds rectangle based on the width and height of the retrieved text
    if (ui->str_table_content) {
        Vector2 text_size = MeasureTextEx(GuiGetFont(), ui->str_table_content, FONT_SIZE, 0);
        ui->sp_table_view.panel_content_bounds.width = text_size.x * 0.9;
        ui->sp_table_view.panel_content_bounds.height = text_size.y / 0.7;
    }

    foodbatch_db_get_all(foodbatch_db); // also prints to stdout
    return;
}

static void process_db_action_in_warning(
    struct ui_food *ui,
    enum error_code *error,
    struct ui_food_db_action_info *action,
    database *foodbatch_db
) {
    switch (action->type) {
    case DB_ACTION_UPDATE:
        if (foodbatch_db_update(
                foodbatch_db,
                action->update.batch_id,
                action->update.name,
                action->update.quantity,
                action->update.is_perishable,
                action->update.date_string,
                action->update.daily_consumption_rate
            )
            != SQLITE_OK)
        {
            *error = ERROR_UPDATE_DB;
            break;
        }
        SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
        break;

    case DB_ACTION_DELETE:
        if (foodbatch_db_delete_by_id(foodbatch_db, action->delete.batch_id) != SQLITE_OK) {
            *error = ERROR_DELETE_DB;
            break;
        }
        SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
        break;

    case DB_ACTION_NONE:
    default:
        break;
    }
}

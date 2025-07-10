/**
 * @file ui_tasks.c
 * @brief Tasks screen implementation
 */
#include "ui/screens/ui_tasks.h"

#include <limits.h> // For INT_MAX
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <external/raylib/raygui.h>

#include "db/tasks_db.h"
#include "global/globals.h"
#include "utils/utilsfn.h"

/* Forward declarations */

static void ui_tasks_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *tasks_db);

static void ui_tasks_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *tasks_db
);

static void ui_tasks_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *tasks_db
);

static void ui_tasks_update_positions(struct ui_base *base);

static void ui_tasks_clear_fields(struct ui_base *base);

static void ui_tasks_cleanup(struct ui_base *base);

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum ui_tasks_db_action_type {
    DB_ACTION_NONE,
    DB_ACTION_DELETE,
};

// Info for the database operation based on the type
struct ui_tasks_db_action_info {
    enum ui_tasks_db_action_type type;
    union {
        struct {
            enum task_status status;
        } delete;
    };
};

static void process_db_action_in_warning(
    struct ui_tasks *ui,
    enum error_code *error,
    struct ui_tasks_db_action_info *action,
    database *tasks_db
);

static void draw_tasks_table_content(Rectangle bounds, char *data);

static void handle_back_button(struct ui_tasks *ui, enum app_state *state);

static void handle_insert_button(struct ui_tasks *ui, enum error_code *error, database *tasks_db);

static void handle_delete_done_button(struct ui_tasks *ui);

static void handle_delete_cancelled_button(struct ui_tasks *ui);

static void handle_view_all_button(struct ui_tasks *ui, database *tasks_db);

/* ======================= PUBLIC FUNCTIONS ======================= */

void ui_tasks_init(struct ui_tasks *ui) {
    // Initialize base
    ui_base_init_defaults(&ui->base, __FILE__);
    // Override methods
    ui->base.render = ui_tasks_render;
    ui->base.handle_buttons = ui_tasks_handle_buttons;
    ui->base.handle_warning_msg = ui_tasks_handle_warning_msg;
    ui->base.update_positions = ui_tasks_update_positions;
    ui->base.clear_fields = ui_tasks_clear_fields;
    ui->base.cleanup = ui_tasks_cleanup;

    // Initialize ui specific fields

    ui->butn_back = button_init((Rectangle) { 20, 20, 0, 30 }, "Back");

    ui->tb_title = textbox_init(
        (Rectangle) { 20, ui->butn_back.bounds.y + ui->butn_back.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Title:"
    );

    ui->tb_desc = textbox_init(
        (Rectangle) { 20, ui->tb_title.bounds.y + ui->tb_title.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Description:"
    );

    ui->due_date_text = (Rectangle) { 20,
                                      ui->tb_desc.bounds.y + ui->tb_desc.bounds.height + (FONT_SIZE * 2),
                                      (float)MeasureText("Expiration date:", FONT_SIZE),
                                      20 };

    ui->ib_year = intbox_init(
        (Rectangle) { 20, ui->due_date_text.y + ui->due_date_text.height + (FONT_SIZE * 2), 40, 30 },
        "Year",
        0,
        9999
    );

    ui->ib_month = intbox_init(
        (Rectangle) { ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5,
                      ui->due_date_text.y + ui->due_date_text.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Month",
        0,
        12
    );

    ui->ib_day = intbox_init(
        (Rectangle) { ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5,
                      ui->due_date_text.y + ui->due_date_text.height + (FONT_SIZE * 2),
                      35,
                      30 },
        "Day",
        0,
        31
    );

    ui->ddb_priority = dropdownbox_init(
        (Rectangle) { 20, ui->ib_year.bounds.y + ui->ib_year.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Low;Normal;High",
        "Priority:"
    );

    ui->ddb_status = dropdownbox_init(
        (Rectangle) { 20, ui->ddb_priority.bounds.y + ui->ddb_priority.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Pending;In Progress;Done;Cancelled",
        "Status:"
    );

    ui->tb_assigned_to = textbox_init(
        (Rectangle) { 20, ui->ddb_status.bounds.y + ui->ddb_status.bounds.height + (FONT_SIZE * 2), 300, 30 },
        "Assigned to:"
    );

    ui->ib_task_id = intbox_init(
        (Rectangle) { 20, ui->tb_assigned_to.bounds.y + ui->tb_assigned_to.bounds.height + (FONT_SIZE * 2), 130, 30 },
        "Entry ID to Update:",
        0,
        INT_MAX
    );

    ui->butn_upsert = button_init((Rectangle) { 20, (float)window_height - 60, 100, 30 }, "Insert/Update");

    ui->butn_delete_status_done = button_init(
        (
            Rectangle
        ) { ui->butn_upsert.bounds.x + ui->butn_upsert.bounds.width + 10, (float)window_height - 60, 100, 30 },
        "Delete all Done entries"
    );

    ui->butn_delete_status_cancelled = button_init(
        (Rectangle) { ui->butn_delete_status_done.bounds.x + ui->butn_delete_status_done.bounds.width + 10,
                      (float)window_height - 60,
                      100,
                      30 },
        "Delete all Cancelled entries"
    );

    ui->butn_view_all = button_init(
        (Rectangle) { ui->butn_delete_status_cancelled.bounds.x + ui->butn_delete_status_cancelled.bounds.width + 10,
                      (float)window_height - 60,
                      100,
                      30 },
        "View All"
    );

    ui->sp_table_view = scrollpanel_init(
        (Rectangle) { ui->tb_title.bounds.x + ui->tb_title.bounds.width + 10,
                      10,
                      (float)window_width - (ui->tb_title.bounds.x + ui->tb_title.bounds.width + 20),
                      (float)window_height - 100 },
        "Database view",
        (Rectangle) { 0, 0, 0, 0 }
    );

    ui->str_table_content = NULL;

    ui->flag = (enum tasks_screen_flags)0;
}

/* ======================= BASE INTERFACE OVERRIDES ======================= */

/**
 * @name UI Base Overrides
 * @brief Implementation of ui_base function pointers
 * @{
 */

/**
 * @brief Tasks screen rendering and interaction handling.
 * 
 * @implements ui_base.render
 *
 * Handles rendering and interaction for all menu elements.
 *
 * @param base Pointer to base UI (implements interface) structure (can be safely cast to any other ui*)
 * @param state Pointer to application state
 * @param error Pointer to error code
 * @param tasks_db Pointer to the tasks database 
 * 
 * @warning Should be called through the base interface
 */
static void ui_tasks_render(struct ui_base *base, enum app_state *state, enum error_code *error, database *tasks_db) {
    struct ui_tasks *ui = (struct ui_tasks *)base;

    textbox_draw(&ui->tb_title);
    textbox_draw(&ui->tb_desc);
    textbox_draw(&ui->tb_assigned_to);

    GuiLabel(ui->due_date_text, "Due Date:");

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

    intbox_draw(&ui->ib_task_id);

    dropdownbox_draw(&ui->ddb_status);
    dropdownbox_draw(&ui->ddb_priority);

    scrollpanel_draw(&ui->sp_table_view, draw_tasks_table_content, ui->str_table_content);

    ui->base.handle_buttons(&ui->base, state, error, tasks_db);
    // Start show warning/error boxes (only if there is a flag set)
    if (ui->flag != 0) {
        ui->base.handle_warning_msg(&ui->base, state, error, tasks_db);
    }

    if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_OPERATION_DONE)) {
        ui->base.clear_fields(&ui->base);
        CLEAR_FLAG(&ui->flag, FLAG_TASKS_OPERATION_DONE);
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
 * @param tasks_db Pointer to tasks database connection
 * 
 * @warning Should be called through the base interface
 */
static void ui_tasks_handle_buttons(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *tasks_db
) {
    struct ui_tasks *ui = (struct ui_tasks *)base;

    if (button_draw_updt(&ui->butn_back)) {
        handle_back_button(ui, state);
        return;
    }

    if (button_draw_updt(&ui->butn_upsert)) {
        handle_insert_button(ui, error, tasks_db);
        return;
    }

    if (button_draw_updt(&ui->butn_delete_status_done)) {
        handle_delete_done_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_delete_status_cancelled)) {
        handle_delete_cancelled_button(ui);
        return;
    }

    if (button_draw_updt(&ui->butn_view_all)) {
        handle_view_all_button(ui, tasks_db);
        return;
    }
}

/**
 * @brief Manages tasks-related warning/confirmation dialogs
 * 
 * @implements ui_base.handle_warning_msg
 * 
 * Shows appropriate warning messages for tasks operations (e.g., deletions),
 * handles user responses, and triggers follow-up actions.
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_tasks*)
 * @param state Pointer to application state
 * @param error Pointer to error tracking variable
 * @param tasks_db Pointer to tasks database connection
 * 
 * @warning May trigger database operations on confirmation
 * 
 */
static void ui_tasks_handle_warning_msg(
    struct ui_base *base,
    enum app_state *state,
    enum error_code *error,
    database *tasks_db
) {
    (void)state;

    struct ui_tasks *ui = (struct ui_tasks *)base;

    const char *message = NULL;
    enum tasks_screen_flags flag_to_clear = (enum tasks_screen_flags)0;
    struct ui_tasks_db_action_info action = { 0 };
    action.type = DB_ACTION_NONE;

    if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_TITLE_EMPTY)) {
        message = "Title must not be empty.";
        flag_to_clear = FLAG_TASKS_TITLE_EMPTY;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_INVALID_DUEDATE)) {
        message = "Date inserted is not valid.";
        flag_to_clear = FLAG_TASKS_INVALID_DUEDATE;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_NOTFOUND)) {
        message = "Task ID not found.";
        flag_to_clear = FLAG_TASKS_NOTFOUND;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_DONE)) {
        message = "Are you sure you want to delete\nall entries with status done?";
        flag_to_clear = FLAG_TASKS_CONFIRM_DELETION_DONE;
        action.type = DB_ACTION_DELETE;
        action.delete.status = TSK_DONE;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_CANCELLED)) {
        message = "Are you sure you want to delete\nall entries with status cancelled?.";
        flag_to_clear = FLAG_TASKS_CONFIRM_DELETION_CANCELLED;
        action.type = DB_ACTION_DELETE;
        action.delete.status = TSK_CANCELLED;
    } else if (IS_FLAG_SET(&ui->flag, FLAG_TASKS_GENERIC_ERROR)) {
        message = "Database error.";
        flag_to_clear = FLAG_TASKS_GENERIC_ERROR;
        *error = NO_ERROR;
    }

    if (message) {
        const char *buttons = (action.type != DB_ACTION_NONE) ? "Yes;No" : "OK";

        int result = GuiMessageBox(
            (Rectangle) { (float)window_width / 2 - 150, (float)window_height / 2 - 50, 300, 150 },
            "#191#Warning!",
            message,
            buttons
        );

        if (result == 1 && action.type != DB_ACTION_NONE) {
            process_db_action_in_warning(ui, error, &action, tasks_db);
        }

        if (result >= 0 && flag_to_clear) {
            CLEAR_FLAG(&ui->flag, flag_to_clear);
        }
    }
}

/**
 * @brief Updates tasks UI element positions for window resizing
 * 
 * @implements ui_base.update_positions
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_tasks*)
 * 
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 * 
 * @warning Should be called on window resize events
 * 
 */
static void ui_tasks_update_positions(struct ui_base *base) {
    struct ui_tasks *ui = (struct ui_tasks *)base;

    ui->butn_upsert.bounds.y = (float)window_height - 60;
    ui->butn_delete_status_done.bounds.y = ui->butn_upsert.bounds.y;
    ui->butn_delete_status_cancelled.bounds.y = ui->butn_upsert.bounds.y;
    ui->butn_view_all.bounds.y = ui->butn_upsert.bounds.y;
    ui->sp_table_view.panel_bounds.width =
        (float)window_width - (ui->tb_title.bounds.x + ui->tb_title.bounds.width + 20);
    ui->sp_table_view.panel_bounds.height = (float)window_height - 100;
}

/**
 * @brief Clears all tasks-related input fields
 * 
 * @implements ui_base.clear_fields
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_tasks*)
 * 
 * @post All text inputs and selections are reset to defaults
 * 
 */
static void ui_tasks_clear_fields(struct ui_base *base) {
    struct ui_tasks *ui = (struct ui_tasks *)base;

    ui->tb_title.input[0] = '\0';
    ui->tb_desc.input[0] = '\0';
    ui->tb_assigned_to.input[0] = '\0';
    ui->ib_task_id.input = 0;
    ui->ib_year.input = 0;
    ui->ib_month.input = 0;
    ui->ib_day.input = 0;
}

/**
 * @brief Cleans up tasks screen resources
 * 
 * @implements ui_base.cleanup
 *
 * @param base Pointer to base UI structure (can be safely cast to ui_tasks*)
 * 
 * @warning Frees any allocated buffers/memory
 * 
 */
static void ui_tasks_cleanup(struct ui_base *base) {
    struct ui_tasks *ui = (struct ui_tasks *)base;

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
 * Handles tasks update, deletion and other DB operations that may be requested
 * through warning message dialogs.
 * 
 * @param ui UI context
 * @param error Error code to set if operation fails
 * @param action Database action to perform with parameters
 * @param tasks_db Database connection
 * 
 */
static void process_db_action_in_warning(
    struct ui_tasks *ui,
    enum error_code *error,
    struct ui_tasks_db_action_info *action,
    database *tasks_db
) {
    switch (action->type) {
    case DB_ACTION_DELETE:
        if (tasks_db_delete_entry_status(tasks_db, action->delete.status) != SQLITE_OK) {
            SET_FLAG(&ui->flag, FLAG_TASKS_GENERIC_ERROR);
            *error = ERROR_DELETE_DB;
            break;
        }
        SET_FLAG(&ui->flag, FLAG_TASKS_OPERATION_DONE);
        break;

    case DB_ACTION_NONE:
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
static void draw_tasks_table_content(Rectangle bounds, char *data) {
    GuiLabel(bounds, data ? data : "No data");
}

static void handle_back_button(struct ui_tasks *ui, enum app_state *state) {
    ui->base.cleanup(&ui->base);
    *state = STATE_MAIN_MENU;
}

static void handle_insert_button(struct ui_tasks *ui, enum error_code *error, database *tasks_db) {
    // Clear possible set flags first
    CLEAR_FLAG(
        &ui->flag,
        FLAG_TASKS_TITLE_EMPTY | FLAG_TASKS_INVALID_DUEDATE | FLAG_TASKS_NOTFOUND | FLAG_TASKS_GENERIC_ERROR
            | FLAG_TASKS_OPERATION_DONE
    );

    // Validate inputs

    bool updating = false;
    // First thing, if updating, check if it exists
    if (ui->ib_task_id.input != 0) {
        if (!tasks_db_check_exists(tasks_db, ui->ib_task_id.input)) {
            fprintf(stderr, "ID not found for update.\n");
            SET_FLAG(&ui->flag, FLAG_TASKS_NOTFOUND);
            return;
        }
        updating = true;
    }

    // At least title must not be empty when inserting
    if (!updating && ui->tb_title.input[0] == '\0') {
        fprintf(stderr, "Title cannot be null.\n");
        SET_FLAG(&ui->flag, FLAG_TASKS_TITLE_EMPTY);
        return;
    }

    // For when due date is different than zero
    char str_due_date[DATE_LEN] = { 0 };
    if (ui->ib_year.input != 0 || ui->ib_month.input != 0 || ui->ib_day.input != 0) {
        if (!validate_date(ui->ib_year.input, ui->ib_month.input, ui->ib_day.input)) {
            fprintf(stderr, "Invalid due date.\n");
            SET_FLAG(&ui->flag, FLAG_TASKS_INVALID_DUEDATE);
            return;
        }
        snprintf(
            str_due_date,
            sizeof(str_due_date),
            "%04d-%02d-%02d",
            ui->ib_year.input,
            ui->ib_month.input,
            ui->ib_day.input
        );
    }

    // For when the status is completed when inserting
    char str_datetime_tsk_done[DATETIME_LEN] = { 0 };
    if (ui->ddb_status.active_option == TSK_DONE) {
        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);

        timeinfo = localtime(&rawtime);
        strftime(str_datetime_tsk_done, sizeof(str_datetime_tsk_done), "%Y-%m-%d %H:%M:%S", timeinfo);
    }

    int rc = tasks_db_upsert(
        tasks_db,
        ui->ib_task_id.input,
        ui->tb_title.input,
        ui->tb_desc.input,
        str_due_date,
        (enum task_priority)ui->ddb_priority.active_option,
        (enum task_status)ui->ddb_status.active_option,
        ui->tb_assigned_to.input,
        str_datetime_tsk_done
    );

    if (rc == SQLITE_NOTFOUND) { // Will never happen as this is checked at the beginning
        fprintf(stderr, "ID not found for update.\n");
        SET_FLAG(&ui->flag, FLAG_TASKS_NOTFOUND);
        return;
    } else if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error.\n");
        SET_FLAG(&ui->flag, FLAG_TASKS_GENERIC_ERROR);
        return;
    }

    SET_FLAG(&ui->flag, FLAG_TASKS_OPERATION_DONE);
    *error = NO_ERROR;
}

static void handle_delete_done_button(struct ui_tasks *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_DONE);
    SET_FLAG(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_DONE);
}

static void handle_delete_cancelled_button(struct ui_tasks *ui) {
    CLEAR_FLAG(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_CANCELLED);
    SET_FLAG(&ui->flag, FLAG_TASKS_CONFIRM_DELETION_CANCELLED);
}

static void handle_view_all_button(struct ui_tasks *ui, database *tasks_db) {
    if (ui->str_table_content) {
        free(ui->str_table_content); // Free old data before getting new data
        ui->str_table_content = NULL;
    }

    int total_tasks = tasks_db_get_count(tasks_db);
    if (total_tasks == -1) {
        fprintf(stderr, "Failed to get total count.\n");
        return;
    }

    // 492 for header + 1029 (at max) for each row
    size_t buffer_size = 512 + 2048 * (size_t)total_tasks;

    ui->str_table_content = malloc(buffer_size);
    if (!ui->str_table_content) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    if (tasks_db_get_all_format(tasks_db, ui->str_table_content, buffer_size) == -1) {
        fprintf(stderr, "Failed to get formatted table.\n");
        free(ui->str_table_content);
        ui->str_table_content = NULL;
        return;
    }

    // Set the panel_content_bounds rectangle based on the width and height of the retrieved text
    if (ui->str_table_content) {
        Vector2 text_size = MeasureTextEx(GuiGetFont(), ui->str_table_content, FONT_SIZE, 0);
        ui->sp_table_view.panel_content_bounds.width = text_size.x * 0.9F;
        ui->sp_table_view.panel_content_bounds.height = text_size.y / 0.7F;
    }

    tasks_db_get_all(tasks_db);
    return;
}

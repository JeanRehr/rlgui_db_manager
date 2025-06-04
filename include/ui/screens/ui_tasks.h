/**
 * @file ui_tasks.h
 * @brief Task Screen Management
 *
 * Handles the presentation and interaction of the application's
 * tasks management interface.
 */

#ifndef UI_TASKS_H
#define UI_TASKS_H

#include "ui/components/button.h"
#include "ui/components/dropdownbox.h"
#include "ui/components/intbox.h"
#include "ui/components/scrollpanel.h"
#include "ui/components/textbox.h"
#include "ui/screens/ui_base.h"

/**
 * @enum tasks_screen_flags
 * @brief State flags for the tasks screen
 *
 * Tracks various states and validation results for the tasks screen.
 */
enum tasks_screen_flags {
    FLAG_TASKS_OPERATION_DONE = 1 << 0,             ///< Operation done
    FLAG_TASKS_GENERIC_ERROR = 1 << 1,              ///< Generic error
    FLAG_TASKS_TITLE_EMPTY = 1 << 2,                ///< Title textbox empty
    FLAG_TASKS_INVALID_DUEDATE = 1 << 3,            ///< Due date invalid entry not found
    FLAG_TASKS_NOTFOUND = 1 << 4,                   ///< ID not found
    FLAG_TASKS_CONFIRM_DELETION_DONE = 1 << 5,      ///< Confirm deletion of all entries with status done
    FLAG_TASKS_CONFIRM_DELETION_CANCELLED = 1 << 6, ///< Confirm deletion of all entries with status cancelled
};

/**
 * @struct ui_tasks
 * @brief Task screen UI components
 *
 * Contains all interactive elements for the tasks management.
 */
struct ui_tasks {
    struct ui_base base; ///< Base ui methods/functionality

    struct textbox tb_title; ///< Title of the task
    struct textbox tb_desc;  ///< Description of the task

    Rectangle due_date_text; ///< Due Date date label bounds
    struct intbox ib_year;   ///< Due Date year input
    struct intbox ib_month;  ///< Due Date month input
    struct intbox ib_day;    ///< Due Date day input

    struct dropdownbox ddb_priority; ///< Priority of the task (should be the same as the enum of tasks.h)
    struct dropdownbox ddb_status;   ///< Status of the task (should be the same as the enum of tasks.h)

    struct textbox tb_assigned_to; ///< Name/username of the assignee

    struct intbox ib_task_id; ///< ID of the task, used for removal/deletion

    struct button butn_back;                    ///< Button to got back to main menu
    struct button butn_upsert;                  ///< Button to insert/update into the database
    struct button butn_delete_status_done;      ///< Button to delete all entries where status=done
    struct button butn_delete_status_cancelled; ///< Button to delete all entries where status=cancelled
    struct button butn_view_all;                ///< Button to get a database view

    struct scrollpanel sp_table_view; ///< A scrollpanel to view the medication database
    char *str_table_content;          ///< The content of the medication database (MUST BE FREED IF ALLOCATED)

    enum tasks_screen_flags flag; ///< Flags for the struct
};

/**
 * @brief Initializes tasks UI elements
 *
 * Sets up all elements with default positions and labels.
 *
 * @param ui Pointer to ui_tasks struct to initialize
 */
void ui_tasks_init(struct ui_tasks *ui);

#endif // UI_TASKS_H

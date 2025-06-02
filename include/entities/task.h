/**
 * @file task.h
 * @brief Task definition for use in database operations/code
 */
#ifndef TASK_H
#define TASK_H

#include "global/CONSTANTS.h"

/**
 * @enum task_priority
 * @brief Task priority enumeration
 * 
 * This enumeration defines task priority options.
 *
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum task_priority { TSK_LOW = 0, TSK_NORMAL, TSK_HIGH, NUM_TASK_PRIORITY };

/**
 * @enum task_status
 * @brief Task status enumeration
 * 
 * This enumeration defines task status options.
 * 
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum task_status { TSK_PENDING = 0, TSK_IN_PROGRESS, TSK_DONE, TSK_CANCELLED, NUM_TASK_STATUS };

/**
 * @addtogroup task_lookup_table
 * 
 * @details If adding a string here, add it in the enum in the same order
 * 
 * @{
 */

/**
 * @brief Task priority lookup
 */
extern const char *task_priority_str[NUM_TASK_PRIORITY];

/**
 * @brief Task status lookup
 */
extern const char *task_status_str[NUM_TASK_STATUS];

/** @} */

/**
 * @struct task
 *
 * @brief Represents a pieace of task record in the database
 *
 * @note To get a string representation of the enums, use the lookup tables
 */
struct task {
    int id;                          ///< ID of the task (get only from the database table)
    char title[MAX_INPUT];           ///< Title of the task
    char description[MAX_INPUT];     ///< Description of the task
    char due_date[DATE_LEN];         ///< ISO 8601 date, e.g., "2024-07-01"
    enum task_priority priority;     ///< Task priority
    enum task_status status;         ///< Task status
    char assigned_to[MAX_INPUT];     ///< User it is assigned to
    char created_at[DATETIME_LEN];   ///< ISO 8601 datetime, e.g., 2025-06-02 15:04:53
    char completed_at[DATETIME_LEN]; ///< ISO 8601 datetime, e.g., 2025-06-02 15:04:53
};

#endif // TASK_H

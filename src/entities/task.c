/**
 * @file task.c
 * @brief This is just to implement the lookup tables, there are no method here (yet)
 */

#include "entities/task.h"

const char *task_priority_str[NUM_TASK_PRIORITY] = { "Low", "Normal", "High" };

const char *task_status_str[NUM_TASK_STATUS] = { "Pending", "In Progress", "Done", "Cancelled" };

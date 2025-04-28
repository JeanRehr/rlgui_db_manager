/**
 * @file ui_create_user.h
 * @brief User Creation Management Screen
 *
 * Provides interface for managing creation of new users, including:
 * - Adding new admin
 * - Updating username
 * - Updating password
 * - @todo add more
 */

#ifndef UI_CREATE_USER_H
#define UI_CREATE_USER_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "ui_elements/button.h"

/**
 * @enum create_user_screen_flags
 * @brief State flags for create user management operations
 *
 * Tracks various states and validation results for user creation management.
 */
enum create_user_screen_flags {
    FLAG_CREATE_USER_OPERATION_DONE = 1 << 0, // 0001: Operation done
};

/**
 * @struct ui_create_user
 * @brief Complete user creation screen state
 *
 * Contains all UI elements and state information needed to manage
 * creation of users.
 */
struct ui_create_user {};

/**
 * @brief Initializes user management screen
 *
 * Sets up all UI elements with default positions and values.
 *
 * @param ui Pointer to ui_create_user struct to initialize
 */
void ui_create_user_init(struct ui_create_user *ui);

/**
 * @brief Draws and manages food management screen
 *
 * Handles rendering and interaction for all screen elements.
 *
 * @param ui Pointer to initialized ui_create_user struct
 * @param state Pointer to application state (may be modified)
 * @param error Pointer to error tracking variable
 * @param user_db Pointer to user database connection
 */
void ui_create_user_draw(struct ui_create_user *ui, enum app_state *state, enum error_code *error, database *user_db);

/**
 * @brief Updates screen element positions
 *
 * Adjusts UI elements based on current window dimensions.
 *
 * @param ui Pointer to ui_create_user struct to update
 *
 * @note If any ui element is initialized with window_width or window_height
 *       in their bounds, they must be updated here
 */
void ui_create_user_updt_pos(struct ui_create_user *ui);

#endif // UI_CREATE_USER_H
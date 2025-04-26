#ifndef UI_CREATE_USER_H
#define UI_CREATE_USER_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "ui_elements/button.h"

enum create_user_screen_flags {
    FLAG_CREATE_USER_OPERATION_DONE = 1 << 0, // 0001: Submission completed
};

// To manage the state of the register food screen
struct ui_create_user {
    Rectangle menu_title_bounds;
};

void ui_create_user_init(struct ui_create_user *ui);

void ui_create_user_draw(struct ui_create_user *ui, enum app_state *state, enum error_code *error, database *user_db);

void ui_create_user_updt_pos(struct ui_create_user *ui);

#endif // UI_CREATE_USER_H
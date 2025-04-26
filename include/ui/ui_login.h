#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "app_state.h"
#include "db/db_manager.h"
#include "error_handling.h"
#include "ui_elements/button.h"
#include "ui_elements/textbox.h"
#include "ui_elements/textboxsecret.h"
#include "user.h"

enum login_screen_flags {
    FLAG_LOGIN_DONE = 1 << 0, // 0001: Operation completed
    FLAG_USER_NOT_EXISTS = 1 << 1,
    FLAG_WRONG_PASSWD = 1 << 2,
    FLAG_USERNAME_EMPTY = 1 << 3,
    FLAG_PASSWD_EMPTY = 1 << 4,
    FLAG_PASSWD_RESET = 1 << 5,
};

// To manage the state of the login menu screen
struct ui_login {
    Rectangle menu_title_bounds;

    struct textbox tb_username;
    struct textboxsecret tbs_password;
    struct button butn_login;

    enum login_screen_flags flag;
};

void ui_login_init(struct ui_login *ui);

void ui_login_draw(
    struct ui_login *ui,
    enum app_state *state,
    enum error_code *error,
    database *user_db,
    struct user *current_user
);

#endif // UI_LOGIN_H
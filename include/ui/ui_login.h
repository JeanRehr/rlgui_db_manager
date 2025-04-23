#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "ui_elements/button.h"
#include "ui_elements/textbox.h"
#include "ui_elements/textboxsecret.h"
#include "app_state.h"
#include "error_handling.h"

enum login_screen_flags {
	FLAG_LOGIN_DONE = 1 << 0, // 0001: Operation completed
    FLAG_USERNAME_NOT_EXISTS = 1 << 1,
    FLAG_WRONG_PASSWD = 1 << 2,
};

// To manage the state of the login menu screen
struct ui_login {
	Rectangle menu_title_bounds;

    struct textbox tb_username;
    struct textboxsecret tbs_password;
    struct button butn_login;
};

void ui_login_init(struct ui_login *ui);

void ui_login_draw(struct ui_login *ui, enum app_state *state, enum error_code *error);

#endif // UI_LOGIN_H
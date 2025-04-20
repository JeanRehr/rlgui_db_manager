#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include "ui_elements/button.h"
#include "app_state.h"
#include "error_handling.h"

// To manage the state of the login menu screen
struct ui_login {
	Rectangle menu_title_bounds;

    struct button butn_login;
};

void ui_login_init(struct ui_login *ui);

void ui_login_draw(struct ui_login *ui, enum app_state *state, enum error_code *error);

#endif // UI_LOGIN_H
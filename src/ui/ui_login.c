#include <external/raygui.h>

#include "ui/ui_login.h"

#include "globals.h"

typedef struct ui_login ui_login;
typedef enum app_state app_state;
typedef enum error_code error_code;

void ui_login_init(ui_login *ui)
{
    ui->menu_title_bounds = (Rectangle) {10, 10, 150, 20};
    ui->butn_login = button_init((Rectangle) {window_width / 2 - 50, window_height / 2 + 100, 100, 30}, "Login");
}

void ui_login_draw(ui_login *ui, app_state *state, error_code *error)
{
    if (button_draw_updt(&ui->butn_login)) {
		*state = STATE_MAIN_MENU;
	}
}
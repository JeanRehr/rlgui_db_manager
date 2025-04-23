#include <external/raylib/raygui.h>

#include "ui/ui_login.h"

#include "globals.h"

typedef struct ui_login ui_login;
typedef enum app_state app_state;
typedef enum error_code error_code;

void ui_login_init(ui_login *ui)
{
	ui->menu_title_bounds = (Rectangle){10, 10, 150, 20};
	ui->tb_username = textbox_init((Rectangle){window_width / 2 - 150, window_height / 2 - 15, 300, 30}, "Username:");
	ui->tbs_password = textboxsecret_init((Rectangle){ui->tb_username.bounds.x, ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2), 300, 30}, "Password:");
	ui->butn_login = button_init((Rectangle){ui->tbs_password.bounds.x + (ui->tbs_password.bounds.width / 2) - 50, ui->tbs_password.bounds.y + (ui->tbs_password.bounds.height * 2), 100, 30}, "Login");
}

void ui_login_draw(ui_login *ui, app_state *state, error_code *error)
{
	// Start draw UI elements

	textbox_draw(&ui->tb_username);

	textboxsecret_draw(&ui->tbs_password);

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_login)) {
		*state = STATE_MAIN_MENU;
	}

	// End button actions

	// Start show warning/error boxes

	// End show warning/error boxes
}
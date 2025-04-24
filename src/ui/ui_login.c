#include <external/raylib/raygui.h>

#include <stdio.h>

#include "ui/ui_login.h"

#include "db/user_db.h"
#include "globals.h"
#include "utilsfn.h"

typedef struct ui_login ui_login;
typedef enum app_state app_state;
typedef enum error_code error_code;

void ui_login_init(ui_login *ui)
{
	ui->menu_title_bounds = (Rectangle){10, 10, 150, 20};
	ui->tb_username = textbox_init((Rectangle){window_width / 2 - 150, window_height / 2 - 15, 300, 30}, "Username:");
	ui->tbs_password = textboxsecret_init((Rectangle){ui->tb_username.bounds.x, ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2), 300, 30}, "Password:");
	ui->butn_login = button_init((Rectangle){ui->tbs_password.bounds.x + (ui->tbs_password.bounds.width / 2) - 50, ui->tbs_password.bounds.y + (ui->tbs_password.bounds.height * 2), 100, 30}, "Login");

	ui->flag = 0;
}

void ui_login_draw(ui_login *ui, app_state *state, error_code *error, database *user_db, struct user *current_user)
{
	// Start draw UI elements

	textbox_draw(&ui->tb_username);

	textboxsecret_draw(&ui->tbs_password);

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_login)) {
		if (*ui->tb_username.input == '\0') {
			fprintf(stderr, "Username is empty.\n");
			SET_FLAG(&ui->flag, FLAG_USERNAME_EMPTY);
		} else if (*ui->tbs_password.input == '\0') {
			fprintf(stderr, "Password is empty.\n");
			SET_FLAG(&ui->flag, FLAG_PASSWD_EMPTY);
		} else if (!user_db_user_exists(user_db, ui->tb_username.input)) {
			fprintf(stderr, "Username does not exists.\n");
			SET_FLAG(&ui->flag, FLAG_USER_NOT_EXISTS);
		} else if (!user_db_authenticate(user_db, ui->tb_username.input, ui->tbs_password.input)) {
			fprintf(stderr, "Password is incorrect.\n");
			SET_FLAG(&ui->flag, FLAG_WRONG_PASSWD);
		} else {
			user_db_get_by_username(user_db, ui->tb_username.input, current_user);
			*state = STATE_MAIN_MENU;
			SET_FLAG(&ui->flag, FLAG_LOGIN_DONE);
		}
	}

	// End button actions

	// Start show warning/error boxes

	if (IS_FLAG_SET(&ui->flag, FLAG_USERNAME_EMPTY)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Username must not be empty.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_USERNAME_EMPTY);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_PASSWD_EMPTY)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Password must not be empty", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_PASSWD_EMPTY);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_USER_NOT_EXISTS)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "User does not exists.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_USER_NOT_EXISTS);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_WRONG_PASSWD)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Password is incorrect.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_WRONG_PASSWD);
		}
	}

	// End show warning/error boxes

	// Clear the text buffer only after a successful operation
	if (IS_FLAG_SET(&ui->flag, FLAG_LOGIN_DONE)) {
		ui->tb_username.input[0] = '\0';
		ui->tbs_password.input[0] = '\0';
		CLEAR_FLAG(&ui->flag, FLAG_LOGIN_DONE);
	}
}
#include <external/raylib/raygui.h>

#include <stdio.h>
#include <string.h>

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
	ui->tbs_password = textboxsecret_init(
		(Rectangle){ui->tb_username.bounds.x, ui->tb_username.bounds.y + (ui->tb_username.bounds.height * 2), 300, 30},
		"Password:");
	ui->butn_login = button_init((Rectangle){ui->tbs_password.bounds.x + (ui->tbs_password.bounds.width / 2) - 50,
											 ui->tbs_password.bounds.y + (ui->tbs_password.bounds.height * 2), 100, 30},
								 "Login");

	ui->flag = 0;
}

void ui_login_draw(ui_login *ui, app_state *state, error_code *error, database *user_db, struct user *current_user)
{
	// Draw UI elements
	textbox_draw(&ui->tb_username);
	textboxsecret_draw(&ui->tbs_password);

	// Handle login button press
	if (button_draw_updt(&ui->butn_login)) {
		handle_login_attempt(ui, state, user_db, current_user);
	}

	// Show warning/error messages
	show_login_messages(ui, state, user_db, current_user);

	// Clear sensitive data after successful login
	if (IS_FLAG_SET(&ui->flag, FLAG_LOGIN_DONE)) {
		clear_data(ui);
		CLEAR_FLAG(&ui->flag, FLAG_LOGIN_DONE);
	}
}

// Helper function to handle login logic
static void handle_login_attempt(ui_login *ui, app_state *state, database *user_db, struct user *current_user)
{
	// Clear previous flags
	CLEAR_FLAG(&ui->flag,
			   FLAG_USERNAME_EMPTY | FLAG_PASSWD_EMPTY | FLAG_USER_NOT_EXISTS | FLAG_WRONG_PASSWD | FLAG_PASSWD_RESET);

	// Validate inputs
	if (ui->tb_username.input[0] == '\0') {
		SET_FLAG(&ui->flag, FLAG_USERNAME_EMPTY);
		return;
	}

	if (ui->tbs_password.input[0] == '\0') {
		SET_FLAG(&ui->flag, FLAG_PASSWD_EMPTY);
		return;
	}

	if (!user_db_user_exists(user_db, ui->tb_username.input)) {
		SET_FLAG(&ui->flag, FLAG_USER_NOT_EXISTS);
		return;
	}

	// Perform authentication
	enum auth_result result = user_db_authenticate(user_db, ui->tb_username.input, ui->tbs_password.input);

	switch (result) {
	case AUTH_NEED_PASSWORD_RESET:
		SET_FLAG(&ui->flag, FLAG_PASSWD_RESET);
		break;

	case AUTH_SUCCESS:
		user_db_get_by_username(user_db, ui->tb_username.input, current_user);
		*state = STATE_MAIN_MENU;
		SET_FLAG(&ui->flag, FLAG_LOGIN_DONE);
		break;

	case AUTH_FAILURE:
	default:
		SET_FLAG(&ui->flag, FLAG_WRONG_PASSWD);
		break;
	}
}

// Helper function to show messages
static void show_login_messages(ui_login *ui, app_state *state, database *user_db, struct user *current_user)
{
	const char *message = NULL;
	enum login_screen_flags *flag_to_clear = NULL;
	bool perform_password_reset = false;

	if (IS_FLAG_SET(&ui->flag, FLAG_USERNAME_EMPTY)) {
		message = "Username must not be empty.";
		flag_to_clear = &ui->flag;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_PASSWD_EMPTY)) {
		message = "Password must not be empty";
		flag_to_clear = &ui->flag;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_USER_NOT_EXISTS | FLAG_WRONG_PASSWD)) {
		message = "Invalid username or password";
		flag_to_clear = &ui->flag;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_PASSWD_RESET)) {
		message = "Please set a new password. The password you entered will become your new password.";
		flag_to_clear = &ui->flag;
		perform_password_reset = true;
	}

	if (message) {
		int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 100},
								   "#191#Warning!", message, "OK");

		if (result >= 0 && flag_to_clear) {
			if (perform_password_reset) {
				// Update the password in the database
				if (user_db_update_password(user_db, ui->tb_username.input, ui->tbs_password.input) == SQLITE_OK) {
					// Log the user in after successful password reset
					user_db_get_by_username(user_db, ui->tb_username.input, current_user);
					*state = STATE_MAIN_MENU;
					SET_FLAG(&ui->flag, FLAG_LOGIN_DONE);
				} else {
					message = "Failed to update password. Please try again.";
					GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 100}, "#191#Warning!",
								  message, "OK");
				}
			}
			*flag_to_clear = 0; // Clear the flag
		}
	}
}

// Helper function to clear sensitive data
static void clear_data(ui_login *ui)
{
	ui->tb_username.input[0] = '\0';
	memset(ui->tbs_password.input, 0, sizeof(ui->tbs_password.input)); // More secure
}
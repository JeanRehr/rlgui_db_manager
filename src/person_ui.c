#include <raygui.h>

#include <stdio.h>

#include <string.h>

#include "person_ui.h"
#include "person.h" // To get the definition and size of a person struct
#include "db_manager.h"
#include "utilsfn.h"
#include "globals.h"

typedef struct person_ui person_ui;
typedef enum app_state app_state;
typedef enum error_code error_code;
typedef enum person_screen_flags person_screen_flags;

void person_ui_init(person_ui *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	ui->butn_back = button_init(
		(Rectangle) {20, ui->menu_title_bounds.y + (ui->menu_title_bounds.height * 2), 0, 30},
		"Back"
	);
	ui->tb_name = textbox_init(
		(Rectangle){20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 300, 30},
		"Name:",
		INPUT_TEXT,
		0
	);
	ui->tb_cpf = textbox_init(
		(Rectangle){20, ui->tb_name.bounds.y + (ui->tb_name.bounds.height * 2), 300, 30},
		"CPF:",
		INPUT_INTEGER,
		11
	);
	ui->ib_age = intbox_init(
		(Rectangle){20, ui->tb_cpf.bounds.y + (ui->tb_cpf.bounds.height * 2), 125, 30},
		"Age:",
		0,
		120
	);
	ui->tb_health_status = textbox_init(
		(Rectangle){20, ui->ib_age.bounds.y + (ui->ib_age.bounds.height * 2), 300, 30},
		"Health Status:",
		INPUT_TEXT,
		0
	);
	ui->tb_needs = textbox_init(
		(Rectangle){20, ui->tb_health_status.bounds.y + (ui->tb_health_status.bounds.height * 2), 300, 30}, 
		"Needs:",
		INPUT_TEXT,
		0
	);

	ui->ddb_gender = dropdownbox_init(
		(Rectangle){20, ui->tb_needs.bounds.y + (ui->tb_needs.bounds.height * 2), 200, 30},
		"Other;Male;Female",
		"Gender"
	);

	ui->butn_submit = button_init((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	ui->butn_retrieve = button_init((Rectangle) {ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	ui->butn_delete = button_init((Rectangle) {ui->butn_retrieve.bounds.x + ui->butn_retrieve.bounds.width + 10, window_height - 100, 100, 30}, "Delete");
	ui->butn_retrieve_all = button_init((Rectangle) {ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, window_height - 100, 0, 30}, "Retrieve All");

	memset(&ui->person_retrieved, 0, sizeof(struct person));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register person screen function
	ui->panel_bounds = (Rectangle) {window_width / 2 - 200, 10, 300, 200};

	ui->flag = 0;
}

void person_ui_draw(person_ui *ui, app_state *state, error_code *error)
{
	// Start draw UI elements

	GuiLabel(ui->menu_title_bounds, "Register Person");

	textbox_draw(&ui->tb_name);
	textbox_draw(&ui->tb_cpf);
	intbox_draw(&ui->ib_age);

	dropdownbox_draw(&ui->ddb_gender);

	textbox_draw(&ui->tb_health_status);
	textbox_draw(&ui->tb_needs);

	// Info Panel
	GuiPanel(ui->panel_bounds, TextFormat("CPF info retrieved: %s", ui->person_retrieved.cpf));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20}, TextFormat("Name: %s", ui->person_retrieved.name));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20}, TextFormat("Age: %d", ui->person_retrieved.age));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20}, TextFormat("Health Status: %.15s...", ui->person_retrieved.health_status));
	GuiLabel((Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20}, "?");

	if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20})) {
		SET_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
	} else {
		CLEAR_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20}, TextFormat("Needs: %.15s...", ui->person_retrieved.needs));
	GuiLabel((Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20}, "?");

	if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20})) {
		SET_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
	} else {
		CLEAR_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20}, TextFormat("Gender: %s", ui->person_retrieved.gender == GENDER_OTHER ? "Other" : ui->person_retrieved.gender == GENDER_MALE ? "Male" : "Female"));
	
	if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_HEALTH)) {
		int dyn_max_input = (int) (MAX_INPUT / 0.9);
		char wrapped_text[dyn_max_input];
		wrap_text(ui->person_retrieved.health_status, wrapped_text, 300);
		GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 300}, "#191#Full Health Status", wrapped_text, "");
	}

	if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_NEEDS)) {
		int dyn_max_input = (int) (MAX_INPUT / 0.9);
		char wrapped_text[dyn_max_input];
		wrap_text(ui->person_retrieved.needs, wrapped_text, 300);
		GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2, 300, 300}, "#191#Full Needs", wrapped_text, "");
	}

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_back)) {
		*state = STATE_MAIN_MENU;
	}

	if (button_draw_updt(&ui->butn_submit)) {
		if (*ui->tb_cpf.input == '\0') {
			SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
			fprintf(stderr, "CPF must not be empty.\n");
		} else if (!is_valid_integer_input(ui->tb_cpf.input, 11, 11)) {
			SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
			fprintf(stderr, "CPF must be 11 digits.\n");
		} else if (db_check_cpf_exists(ui->tb_cpf.input)) {
			SET_FLAG(&ui->flag, FLAG_CPF_EXISTS);
		} else if (db_insert_person(ui->tb_cpf.input, ui->tb_name.input, ui->ib_age.input, ui->tb_health_status.input, ui->tb_needs.input, ui->ddb_gender.active_option) != SQLITE_OK) {
			*error = ERROR_INSERT_DB;
			fprintf(stderr, "Error submitting to database.\n");
		} else {
			*error = NO_ERROR;
			SET_FLAG(&ui->flag, FLAG_PERSON_OPERATION_DONE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve)) {
		if (db_get_person_by_cpf(ui->tb_cpf.input, &ui->person_retrieved)) {
			printf("Retrieved Person - Name: %s, Age: %d, Health Status: %s, Needs: %s, Gender: %d\n", ui->person_retrieved.name, ui->person_retrieved.age, ui->person_retrieved.health_status, ui->person_retrieved.needs, ui->person_retrieved.gender);
			SET_FLAG(&ui->flag, FLAG_PERSON_OPERATION_DONE);
		} else {
			SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
		}
	}

	if (button_draw_updt(&ui->butn_delete)) {
		if (*ui->tb_cpf.input == '\0') {
			SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
		} else if (!is_valid_integer_input(ui->tb_cpf.input, 11, 11)) {
			SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
		} else if (!db_check_cpf_exists(ui->tb_cpf.input)) {
			SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
		} else {
			SET_FLAG(&ui->flag, FLAG_CONFIRM_PERSON_DELETE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve_all)) {
		db_get_all_persons();
	}

	// End button actions

	// Start show warning/error boxes

	// In case updating person
	if (IS_FLAG_SET(&ui->flag, FLAG_CPF_EXISTS)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF Already exists.", "Update;Don't update");
		if (result == 1) {
			if (db_update_person(ui->tb_cpf.input, ui->tb_name.input, ui->ib_age.input, ui->tb_health_status.input, ui->tb_needs.input, ui->ddb_gender.active_option) != SQLITE_OK) {
				*error = ERROR_UPDATE_DB;
			}
		}
		if (result >= 0) {
			*error = NO_ERROR;
			CLEAR_FLAG(&ui->flag, FLAG_CPF_EXISTS);
			SET_FLAG(&ui->flag, FLAG_PERSON_OPERATION_DONE);
		}
	}

	// In case deleting person
	if (IS_FLAG_SET(&ui->flag, FLAG_CONFIRM_PERSON_DELETE)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Deleting Person!", "Are you sure you want to delete?", "Yes, delete;NO");
		if (result == 1) {
			db_delete_person_by_cpf(ui->tb_cpf.input);
			SET_FLAG(&ui->flag, FLAG_PERSON_OPERATION_DONE);
		}
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_CONFIRM_PERSON_DELETE);
		}
	}

	// Warnings
	if (IS_FLAG_SET(&ui->flag, FLAG_INPUT_CPF_EMPTY)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must not be empty.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_VALID)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must be 11 digits.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_FOUND)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF not found.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
		}
	} else if (*error == ERROR_INSERT_DB || *error == ERROR_UPDATE_DB) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Error submitting to database.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	}

	// End show warning/error boxes

	// Clear the text buffer only after a successful operation
	if (IS_FLAG_SET(&ui->flag, FLAG_PERSON_OPERATION_DONE)) {
		ui->tb_name.input[0] = '\0';
		ui->tb_cpf.input[0] = '\0';
		ui->ib_age.input = 0;
		ui->tb_health_status.input[0] = '\0';
		ui->tb_needs.input[0] = '\0';
		CLEAR_FLAG(&ui->flag, FLAG_PERSON_OPERATION_DONE);
	}
}

void person_ui_updt_pos(person_ui *ui)
{
	ui->butn_submit.bounds.y = window_height - 50;
	ui->butn_retrieve.bounds.y = window_height - 50;
	ui->butn_delete.bounds.y = window_height - 50;
	ui->butn_retrieve_all.bounds.y = window_height - 50;
	ui->panel_bounds.x = window_width / 2 - 150;
}
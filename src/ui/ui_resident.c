#include <external/raylib/raygui.h>

#include <stdio.h>

#include <string.h>

#include "ui/ui_resident.h"
#include "resident.h" // To get the definition and size of a resident struct
#include "db/resident_db.h"
#include "utilsfn.h"
#include "globals.h"

// Tagged union for when a warning message needs to perform a database operation
// Type of the operation
enum db_action_type {
	DB_ACTION_NONE,
	DB_ACTION_UPDATE,
	DB_ACTION_DELETE,
} db_action_type;

// Info for the database operation based on the type
struct db_action_info {
	enum db_action_type type;
	union {
		struct {
			const char* cpf;
			const char* name;
			int age;
			const char* health_status;
			const char* needs;
			bool medical_assistance;
			int gender;
		} update;

		struct {
			const char* cpf;
		} delete;
	};
} db_action_info;

// Helper function prototypes
static void draw_resident_info_panel(struct ui_resident *ui);
static void handle_button_actions(struct ui_resident *ui, enum app_state *state, enum error_code *error, database *resident_db);
static void handle_submit_action(struct ui_resident *ui, enum error_code *error, database *resident_db);
static void handle_retrieve_action(struct ui_resident *ui, database *resident_db);
static void handle_delete_action(struct ui_resident *ui, database *resident_db);
static void show_warning_messages(struct ui_resident *ui, enum error_code *error, database *resident_db);
static void process_action_in_warning(struct ui_resident *ui, enum error_code *error, struct db_action_info *action, database *resident_db);
static void clear_input_fields(struct ui_resident *ui);

void ui_resident_init(struct ui_resident *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	ui->butn_back = button_init(
		(Rectangle) {20, ui->menu_title_bounds.y + (ui->menu_title_bounds.height * 2), 0, 30},
		"Back"
	);
	ui->tb_name = textbox_init(
		(Rectangle){20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 300, 30},
		"Name:"
	);
	ui->tb_cpf = textboxint_init(
		(Rectangle){20, ui->tb_name.bounds.y + (ui->tb_name.bounds.height * 2), 300, 30},
		"CPF:"
	);
	ui->ib_age = intbox_init(
		(Rectangle){20, ui->tb_cpf.bounds.y + (ui->tb_cpf.bounds.height * 2), 125, 30},
		"Age:",
		0,
		120
	);
	ui->tb_health_status = textbox_init(
		(Rectangle){20, ui->ib_age.bounds.y + (ui->ib_age.bounds.height * 2), 300, 30},
		"Health Status:"
	);
	ui->tb_needs = textbox_init(
		(Rectangle){20, ui->tb_health_status.bounds.y + (ui->tb_health_status.bounds.height * 2), 300, 30}, 
		"Needs:"
	);

	ui->cb_medical_assistance = checkbox_init(
		(Rectangle){20, ui->tb_needs.bounds.y + (ui->tb_needs.bounds.height * 2), 20, 20},
		"Need Medical Assistance:"
	);

	ui->ddb_gender = dropdownbox_init(
		(Rectangle){20, ui->cb_medical_assistance.bounds.y + (ui->cb_medical_assistance.bounds.height * 2), 200, 30},
		"Other;Male;Female",
		"Gender"
	);

	ui->butn_submit = button_init((Rectangle) {20, window_height - 60, 100, 30}, "Submit");
	ui->butn_retrieve = button_init((Rectangle) {ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30}, "Retrieve");
	ui->butn_delete = button_init((Rectangle) {ui->butn_retrieve.bounds.x + ui->butn_retrieve.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30}, "Delete");
	ui->butn_retrieve_all = button_init((Rectangle) {ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, ui->butn_submit.bounds.y, 0, 30}, "Retrieve All");

	memset(&ui->resident_retrieved, 0, sizeof(struct resident));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register resident screen function
	ui->panel_bounds = (Rectangle) {window_width / 2 - 150, 10, 300, 250};

	ui->flag = 0;
}

void ui_resident_draw(struct ui_resident *ui, enum app_state *state, enum error_code *error, database *resident_db)
{
	// Start draw UI elements

	GuiLabel(ui->menu_title_bounds, "Register Person");

	textbox_draw(&ui->tb_name);
	textboxint_draw(&ui->tb_cpf);
	intbox_draw(&ui->ib_age);

	dropdownbox_draw(&ui->ddb_gender);

	textbox_draw(&ui->tb_health_status);
	textbox_draw(&ui->tb_needs);

	checkbox_draw(&ui->cb_medical_assistance);

	// End draw UI elements

	// Draw info panel
	draw_resident_info_panel(ui);

	// Handle button actions
	handle_button_actions(ui, state, error, resident_db);

	// Show warning/error messages
	show_warning_messages(ui, error, resident_db);

	// Clear fields after successful operation
	if (IS_FLAG_SET(&ui->flag, FLAG_RESIDENT_OPERATION_DONE)) {
		clear_input_fields(ui);
		CLEAR_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
	}
}

static void draw_resident_info_panel(struct ui_resident *ui)
{
	GuiPanel(ui->panel_bounds, TextFormat("CPF info retrieved: %s", ui->resident_retrieved.cpf));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20}, TextFormat("Name: %s", ui->resident_retrieved.name));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20}, TextFormat("Age: %d", ui->resident_retrieved.age));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20}, TextFormat("Health Status: %.15s...", ui->resident_retrieved.health_status));
	GuiLabel((Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20}, "?");

	if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 90, 20, 20})) {
		SET_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
	} else {
		CLEAR_FLAG(&ui->flag, FLAG_SHOW_HEALTH);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20}, TextFormat("Needs: %.15s...", ui->resident_retrieved.needs));
	GuiLabel((Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20}, "?");

	if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ui->panel_bounds.x + (ui->panel_bounds.width - 30), ui->panel_bounds.y + 120, 20, 20})) {
		SET_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
	} else {
		CLEAR_FLAG(&ui->flag, FLAG_SHOW_NEEDS);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20}, TextFormat("Need Medical Assistance: %s", ui->resident_retrieved.medical_assistance == true ? "True" : "False"));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 180, 280, 20}, TextFormat("Gender: %s", ui->resident_retrieved.gender == GENDER_OTHER ? "Other" : ui->resident_retrieved.gender == GENDER_MALE ? "Male" : "Female"));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 210, 280, 20}, TextFormat("Entry Date: %s", ui->resident_retrieved.entry_date));
	
	if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_HEALTH)) {
		int dyn_max_input = (int) (MAX_INPUT / 0.9);
		char wrapped_text[dyn_max_input];
		wrap_text(ui->resident_retrieved.health_status, wrapped_text, 300);
		GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 300}, "#191#Full Health Status", wrapped_text, "");
	}

	if (IS_FLAG_SET(&ui->flag, FLAG_SHOW_NEEDS)) {
		int dyn_max_input = (int) (MAX_INPUT / 0.9);
		char wrapped_text[dyn_max_input];
		wrap_text(ui->resident_retrieved.needs, wrapped_text, 300);
		GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2, 300, 300}, "#191#Full Needs", wrapped_text, "");
	}
}

static void handle_button_actions(struct ui_resident *ui, enum app_state *state, enum error_code *error, database *resident_db)
{
	if (button_draw_updt(&ui->butn_back)) {
		*state = STATE_MAIN_MENU;
		return;
	}

	if (button_draw_updt(&ui->butn_submit)) {
		handle_submit_action(ui, error, resident_db);
	}

	if (button_draw_updt(&ui->butn_retrieve)) {
		handle_retrieve_action(ui, resident_db);
	}

	if (button_draw_updt(&ui->butn_delete)) {
		handle_delete_action(ui, resident_db);
	}

	if (button_draw_updt(&ui->butn_retrieve_all)) {
		resident_db_get_all(resident_db);
	}
}

static void handle_submit_action(struct ui_resident *ui, enum error_code *error, database *resident_db)
{
	// Clear previous flags
	CLEAR_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY | FLAG_CPF_NOT_VALID | FLAG_CPF_EXISTS);

	// Validate inputs
	if (ui->tb_cpf.input[0] == '\0') {
		SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
		return;
	}

	if (!is_int_between_min_max(ui->tb_cpf.input, 11, 11)) {
		SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
		return;
	}

	// Check if CPF exists		
	if (resident_db_check_cpf_exists(resident_db, ui->tb_cpf.input)) {
		SET_FLAG(&ui->flag, FLAG_CPF_EXISTS);
		return;
	}

	// Insert new resident
	if (resident_db_insert(resident_db, ui->tb_cpf.input, ui->tb_name.input, 
						  ui->ib_age.input, ui->tb_health_status.input, 
						  ui->tb_needs.input, ui->cb_medical_assistance.checked, 
						  ui->ddb_gender.active_option) != SQLITE_OK) {
		*error = ERROR_INSERT_DB;
		return;
	}

	SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
	*error = NO_ERROR;
}

static void handle_retrieve_action(struct ui_resident *ui, database *resident_db)
{
	CLEAR_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);

	if (resident_db_get_by_cpf(resident_db, ui->tb_cpf.input, &ui->resident_retrieved) == SQLITE_OK) {
		printf("Retrieved Person - Name: %s, Age: %d\n", ui->resident_retrieved.name, ui->resident_retrieved.age);
		SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
	} else {
		SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
	}
}

static void handle_delete_action(struct ui_resident *ui, database *resident_db)
{
	CLEAR_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY | FLAG_CPF_NOT_VALID | FLAG_CPF_NOT_FOUND | FLAG_CONFIRM_RESIDENT_DELETE);

	if (ui->tb_cpf.input[0] == '\0') {
		SET_FLAG(&ui->flag, FLAG_INPUT_CPF_EMPTY);
		return;
	}

	if (!is_int_between_min_max(ui->tb_cpf.input, 11, 11)) {
		SET_FLAG(&ui->flag, FLAG_CPF_NOT_VALID);
		return;
	}

	if (!resident_db_check_cpf_exists(resident_db, ui->tb_cpf.input)) {
		SET_FLAG(&ui->flag, FLAG_CPF_NOT_FOUND);
		return;
	}

	SET_FLAG(&ui->flag, FLAG_CONFIRM_RESIDENT_DELETE);
}

static void show_warning_messages(struct ui_resident *ui, enum error_code *error, database *resident_db)
{
	const char *message = NULL;
	enum resident_screen_flags flag_to_clear = 0;
	struct db_action_info action = {DB_ACTION_NONE};

	if (IS_FLAG_SET(&ui->flag, FLAG_INPUT_CPF_EMPTY)) {
		message = "CPF must not be empty.";
		flag_to_clear = FLAG_INPUT_CPF_EMPTY;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_VALID)) {
		message = "CPF must be 11 digits.";
		flag_to_clear = FLAG_CPF_NOT_VALID;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_NOT_FOUND)) {
		message = "CPF not found.";
		flag_to_clear = FLAG_CPF_NOT_FOUND;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CPF_EXISTS)) {
		message = "CPF already exists. Update existing record?";
		flag_to_clear = FLAG_CPF_EXISTS;
		action.type = DB_ACTION_UPDATE;
		action.update.cpf = ui->tb_cpf.input;
		action.update.name = ui->tb_name.input;
		action.update.age = ui->ib_age.input;
		action.update.health_status = ui->tb_health_status.input;
		action.update.needs = ui->tb_needs.input;
		action.update.medical_assistance = ui->cb_medical_assistance.checked;
		action.update.gender = ui->ddb_gender.active_option;
	} else if (IS_FLAG_SET(&ui->flag, FLAG_CONFIRM_RESIDENT_DELETE)) {
		message = "Are you sure you want to delete resident?";
		flag_to_clear = FLAG_CONFIRM_RESIDENT_DELETE;
		action.type = DB_ACTION_DELETE;
		action.delete.cpf = ui->tb_cpf.input;
	} else if (*error == ERROR_INSERT_DB || *error == ERROR_UPDATE_DB) {
		message = "Error submitting to database.";
		*error = NO_ERROR; // Clear error after showing
	}

	if (message) {
		const char* buttons = (action.type != DB_ACTION_NONE) ? "Yes;No" : "OK";
		int result = GuiMessageBox(
			(Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 100},
			"#191#Warning!",
			message,
			buttons
		);

		if (result == 1 && action.type != DB_ACTION_NONE) {
			process_db_action_in_warning(ui, error, &action, resident_db);
		}

		if (result >= 0 && flag_to_clear) {
			CLEAR_FLAG(&ui->flag, flag_to_clear);
		}
	}
}

static void process_db_action_in_warning(struct ui_resident *ui, enum error_code *error, struct db_action_info *action, database *resident_db)
{
	switch (action->type) {
	case DB_ACTION_UPDATE:
		if (resident_db_update(resident_db, action->update.cpf,
							 action->update.name,
							 action->update.age,
							 action->update.health_status,
							 action->update.needs,
							 action->update.medical_assistance,
							 action->update.gender) == SQLITE_OK) {
			SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
		} else {
			*error = ERROR_UPDATE_DB;
		}
		break;

	case DB_ACTION_DELETE:
		if (resident_db_delete_by_cpf(resident_db, action->delete.cpf) == SQLITE_OK) {
			SET_FLAG(&ui->flag, FLAG_RESIDENT_OPERATION_DONE);
		}
		break;

	case DB_ACTION_NONE:
	default:
		break;
	}
}

static void clear_input_fields(struct ui_resident *ui)
{
	ui->tb_name.input[0] = '\0';
	ui->tb_cpf.input[0] = '\0';
	ui->ib_age.input = 0;
	ui->tb_health_status.input[0] = '\0';
	ui->tb_needs.input[0] = '\0';
	ui->cb_medical_assistance.checked = false;
	ui->ddb_gender.active_option = 0;
}

void ui_resident_updt_pos(struct ui_resident *ui)
{
	ui->butn_submit.bounds.y = window_height - 60;
	ui->butn_retrieve.bounds.y = ui->butn_submit.bounds.y;
	ui->butn_delete.bounds.y = ui->butn_submit.bounds.y;
	ui->butn_retrieve_all.bounds.y = ui->butn_submit.bounds.y;
	ui->panel_bounds.x = window_width / 2 - 150;
}
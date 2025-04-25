#include <external/raylib/raygui.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "ui/ui_food.h"
#include "food.h" // To get the definition and size of a foodbatch struct
#include "db/foodbatch_db.h"
#include "globals.h"
#include "utilsfn.h"

typedef struct ui_food ui_food;
typedef enum app_state app_state;
typedef enum error_code error_code;
typedef enum food_screen_flags food_screen_flags;

void ui_food_init(ui_food *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 150, 20};

	ui->butn_back = button_init(
		(Rectangle) {20, ui->menu_title_bounds.y + (ui->menu_title_bounds.height * 2), 0, 30},
		"Back"
	);

	ui->ib_batch_id = intbox_init(
		(Rectangle){20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 130, 30},
		"Batch ID:",
		0,
		99999999
	);
	ui->tb_name = textbox_init(
		(Rectangle){20, ui->ib_batch_id.bounds.y + (ui->ib_batch_id.bounds.height * 2), 300, 30},
		"Food Name:"
	);
	ui->ib_quantity = intbox_init(
		(Rectangle){20, ui->tb_name.bounds.y + (ui->tb_name.bounds.height * 2), 125, 30},
		"Quantity:",
		0,
		INT_MAX
	);

	ui->cb_is_perishable = checkbox_init(
		(Rectangle){20, ui->ib_quantity.bounds.y + (ui->ib_quantity.bounds.height * 2), 20, 20}, 
		"Is Perishable?"
	);

	ui->expirationDateText = (Rectangle) {20, ui->cb_is_perishable.bounds.y + (ui->cb_is_perishable.bounds.height * 2), MeasureText("Expiration date:", FONT_SIZE), 20};

	ui->ib_year = intbox_init(
		(Rectangle) {20, ui->expirationDateText.y + (ui->expirationDateText.height * 2), 40, 30}, "Year", 0, 9999
	);

	ui->ib_month = intbox_init(
		(Rectangle) {ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5, ui->expirationDateText.y + (ui->expirationDateText.height * 2), 35, 30}, "Month", 0, 12
	);

	ui->ib_day = intbox_init(
		(Rectangle) {ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5, ui->expirationDateText.y + (ui->expirationDateText.height * 2), 35, 30}, "Day", 0, 31
	);

	ui->fb_daily_consumption_rate = floatbox_init(
		(Rectangle){20, ui->ib_year.bounds.y + (ui->ib_year.bounds.height * 2), 250, 30},
		"Avg daily consumption rate per resident?"
	);

	ui->butn_submit = button_init((Rectangle) {20, window_height - 60, 100, 30}, "Submit");
	ui->butn_retrieve = button_init((Rectangle) {ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30}, "Retrieve");
	ui->butn_delete = button_init((Rectangle) {ui->butn_retrieve.bounds.x + ui->butn_retrieve.bounds.width + 10, ui->butn_submit.bounds.y, 100, 30}, "Delete");
	ui->butn_retrieve_all = button_init((Rectangle) {ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, ui->butn_submit.bounds.y, 0, 30}, "Retrieve All");

	memset(&ui->foodbatch_retrieved, 0, sizeof(struct foodbatch));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register resident screen function
	ui->panel_bounds = (Rectangle) {window_width / 2 - 150, 10, 300, 200};

	ui->flag = 0;
}

void ui_food_draw(ui_food *ui, app_state *state, error_code *error, database *foodbatch_db)
{
	// Start draw UI elements

	GuiLabel(ui->menu_title_bounds, "Register Food Batch");

	intbox_draw(&ui->ib_batch_id);
	textbox_draw(&ui->tb_name);
	intbox_draw(&ui->ib_quantity);

	checkbox_draw(&ui->cb_is_perishable);

	GuiLabel(ui->expirationDateText, "Expiration date:");

	intbox_draw(&ui->ib_year);
	GuiLabel((Rectangle){ui->ib_year.bounds.x + ui->ib_year.bounds.width - 1, ui->ib_year.bounds.y + (ui->ib_year.bounds.height / 2) - 5, 10, 10}, "-");
	intbox_draw(&ui->ib_month);
	GuiLabel((Rectangle){ui->ib_month.bounds.x + ui->ib_month.bounds.width - 1, ui->ib_month.bounds.y + (ui->ib_month.bounds.height / 2) - 5, 10, 10}, "-");
	intbox_draw(&ui->ib_day);

	if (ui->cb_is_perishable.checked == false && (ui->ib_year.input != 1 || ui->ib_month.input != 1 || ui->ib_day.input != 1)) {
		// Setting a default value for inserting into the database
		ui->ib_year.input = 1;
		ui->ib_month.input = 1;
		ui->ib_day.input = 1;
	}

	floatbox_draw(&ui->fb_daily_consumption_rate);

	// Start Info Panel
	GuiPanel(ui->panel_bounds, TextFormat("Batch ID retrieved: %d", ui->foodbatch_retrieved.batch_id));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20}, TextFormat("Name: %s", ui->foodbatch_retrieved.name));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20}, TextFormat("Quantity: %d", ui->foodbatch_retrieved.quantity));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20}, TextFormat("Expiration date: %s", ui->foodbatch_retrieved.expiration_date));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20}, TextFormat("Is Perishable: %s", ui->foodbatch_retrieved.is_perishable == true ? "True" : "False"));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20}, TextFormat("Daily Consumption Rate: %.2f", ui->foodbatch_retrieved.daily_consumption_rate));

	// End Info Panel

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_back)) {
		*state = STATE_MAIN_MENU;
	}

	if (button_draw_updt(&ui->butn_submit)) {
		bool is_valid_date = validate_date(ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);
		char date_string[11] = {0}; // YYYY-MM-DD + null terminator
		snprintf(date_string, sizeof(date_string), "%04d-%02d-%02d", ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);
		if (!is_valid_date) {
			printf("Date not valid, year: %d, month: %d, day: %d\n", ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);
			SET_FLAG(&ui->flag, FLAG_INVALID_FOOD_DATE);
		} else if (foodbatch_db_check_batchid_exists(foodbatch_db, ui->ib_batch_id.input)) {
			SET_FLAG(&ui->flag, FLAG_BATCHID_EXISTS);
			printf("Food batch ID exists: %d\n", ui->ib_batch_id.input);
		} else if (foodbatch_db_insert(foodbatch_db, ui->ib_batch_id.input, ui->tb_name.input, ui->ib_quantity.input, ui->cb_is_perishable.checked, date_string, ui->fb_daily_consumption_rate.value) != SQLITE_OK) {
			*error = ERROR_INSERT_DB;
			fprintf(stderr, "Error submitting to database.\n");
		} else {
			*error = NO_ERROR;
			SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve)) {
		if (foodbatch_db_get_by_batchid(foodbatch_db, ui->ib_batch_id.input, &ui->foodbatch_retrieved) == SQLITE_OK) {
			printf("Retrieved Food Batch - Name: %s, Qauntity: %d, Is Perishable: %d, Expiration Date: %s, Daily Consumption Rate: %f\n", ui->foodbatch_retrieved.name, ui->foodbatch_retrieved.quantity, ui->foodbatch_retrieved.is_perishable, ui->foodbatch_retrieved.expiration_date, ui->foodbatch_retrieved.daily_consumption_rate);
			SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
		} else {
			SET_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);
		}
	}

	if (button_draw_updt(&ui->butn_delete)) {
		if (!foodbatch_db_check_batchid_exists(foodbatch_db, ui->ib_batch_id.input)) {
			SET_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);
		} else {
			SET_FLAG(&ui->flag, FLAG_CONFIRM_FOOD_DELETE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve_all)) {
		foodbatch_db_get_all(foodbatch_db);
	}

	// End button actions

	// Start show warning/error boxes
	
	// In case updating food batch
	if (IS_FLAG_SET(&ui->flag, FLAG_BATCHID_EXISTS)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Batch ID already exists.", "Update;Don't update");
		if (result == 1) {
			// Don't need to validate date here, as it's impossible to be wrong at this stage
			char date_string[11] = {0}; // YYYY-MM-DD + null terminator
			snprintf(date_string, sizeof(date_string), "%04d-%02d-%02d", ui->ib_year.input, ui->ib_month.input, ui->ib_day.input);
			if (foodbatch_db_update(foodbatch_db, ui->ib_batch_id.input, ui->tb_name.input, ui->ib_quantity.input, ui->cb_is_perishable.checked, date_string, ui->fb_daily_consumption_rate.value) != SQLITE_OK) {
				*error = ERROR_UPDATE_DB;
			}
		}
		if (result >= 0) {
			*error = NO_ERROR;
			CLEAR_FLAG(&ui->flag, FLAG_BATCHID_EXISTS);
			SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
		}
	}

	// In case deleting food batch
	if (IS_FLAG_SET(&ui->flag, FLAG_CONFIRM_FOOD_DELETE)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Deleting Person!", "Are you sure you want to delete?", "Yes, delete;NO");
		if (result == 1) {
			foodbatch_db_delete_by_id(foodbatch_db ,ui->ib_batch_id.input);
			SET_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
		}
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_CONFIRM_FOOD_DELETE);
		}
	}

	// Warnings
	if (IS_FLAG_SET(&ui->flag, FLAG_BATCHID_NOT_FOUND)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Batch ID not found.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_BATCHID_NOT_FOUND);
		}
	} else if (IS_FLAG_SET(&ui->flag, FLAG_INVALID_FOOD_DATE)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Date inserted is not valid.", "OK");
		if (result >= 0) {
			CLEAR_FLAG(&ui->flag, FLAG_INVALID_FOOD_DATE);
		}
	} else if (*error == ERROR_INSERT_DB) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Error submitting to database.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	}

	// In case updating food batch

	// End show warning/error boxes

	// Clear the text buffer only after a successful operation
	if (IS_FLAG_SET(&ui->flag, FLAG_FOOD_OPERATION_DONE)) {
		ui->ib_batch_id.input = 0;
		ui->tb_name.input[0] = '\0';
		ui->ib_quantity.input = 0;
		ui->cb_is_perishable.checked = false;
		ui->ib_year.input = 0;
		ui->ib_month.input = 0;
		ui->ib_day.input = 0;
		ui->fb_daily_consumption_rate.value = 0;
		ui->fb_daily_consumption_rate.text_input[0] = '\0';
		CLEAR_FLAG(&ui->flag, FLAG_FOOD_OPERATION_DONE);
	}

	// Warnings
	// End show warning/error boxes
}

void ui_food_updt_pos(struct ui_food *ui)
{
	ui->butn_submit.bounds.y = window_height - 60;
	ui->butn_retrieve.bounds.y = ui->butn_submit.bounds.y;
	ui->butn_delete.bounds.y = ui->butn_submit.bounds.y;
	ui->butn_retrieve_all.bounds.y = ui->butn_submit.bounds.y;
	ui->panel_bounds.x = window_width / 2 - 150;
}
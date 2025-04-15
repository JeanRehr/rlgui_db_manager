#include <raygui.h>

#include <string.h>
#include <limits.h>

#include "food_ui.h"
#include "food.h" // To get the definition and size of a foodbatch struct
#include "db_manager.h"
#include "globals.h"

typedef struct food_ui food_ui;
typedef enum app_state app_state;
typedef enum error_code error_code;
typedef enum food_screen_flags food_screen_flags;

void food_ui_init(food_ui *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 150, 20};

	ui->butn_back = button_init(
		(Rectangle) {20, ui->menu_title_bounds.y + (ui->menu_title_bounds.height * 2), 0, 30},
		"Back"
	);

	ui->ib_batch_id = intbox_init(
		(Rectangle){20, ui->butn_back.bounds.y + (ui->butn_back.bounds.height * 2), 130, 30},
		"Batch ID:",
		1,
		99999999
	);
	ui->tb_name = textbox_init(
		(Rectangle){20, ui->ib_batch_id.bounds.y + (ui->ib_batch_id.bounds.height * 2), 300, 30},
		"Food Name:",
		INPUT_TEXT,
		0
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


	ui->ib_year = intbox_init(
		(Rectangle) {20, ui->cb_is_perishable.bounds.y + (ui->cb_is_perishable.bounds.height * 2), 40, 30}, "Year", 1000, 2025
	);

	ui->ib_month = intbox_init(
		(Rectangle) {ui->ib_year.bounds.x + ui->ib_year.bounds.width + 5, ui->cb_is_perishable.bounds.y + (ui->cb_is_perishable.bounds.height * 2), 35, 30}, "Month", 1, 12
	);

	ui->ib_day = intbox_init(
		(Rectangle) {ui->ib_month.bounds.x + ui->ib_month.bounds.width + 5, ui->cb_is_perishable.bounds.y + (ui->cb_is_perishable.bounds.height * 2), 35, 30}, "Day", 1, 31
	);

	ui->fb_daily_consumption_rate = floatbox_init(
		(Rectangle){20, ui->ib_year.bounds.y + (ui->ib_year.bounds.height * 2), 250, 30},
		"Avg daily consumption rate per person?"
	);

	ui->butn_submit = button_init((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	ui->butn_retrieve = button_init((Rectangle) {ui->butn_submit.bounds.x + ui->butn_submit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	ui->butn_delete = button_init((Rectangle) {ui->butn_retrieve.bounds.x + ui->butn_retrieve.bounds.width + 10, window_height - 100, 100, 30}, "Delete");
	ui->butn_retrieve_all = button_init((Rectangle) {ui->butn_delete.bounds.x + ui->butn_delete.bounds.width + 10, window_height - 100, 0, 30}, "Retrieve All");

	memset(&ui->foodbatch_retrieved, 0, sizeof(struct foodbatch));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register person screen function
	ui->panel_bounds = (Rectangle) {window_width / 2 - 200, 10, 300, 200};

	ui->flag = 0;
}

void food_ui_draw(food_ui *ui, app_state *state, error_code *error)
{
	// Start draw UI elements

	GuiLabel(ui->menu_title_bounds, "Register Food Batch");

	intbox_draw(&ui->ib_batch_id);
	textbox_draw(&ui->tb_name);
	intbox_draw(&ui->ib_quantity);

	checkbox_draw(&ui->cb_is_perishable);

	if (ui->cb_is_perishable.checked == true) {
		// if checkbox for perishable is checked, set the intboxes for expiration date as readonly
		GuiTextBox(ui->ib_year.bounds, "0", MAX_INPUT, false);
		GuiLabel((Rectangle){ui->ib_year.bounds.x + ui->ib_year.bounds.width - 1, ui->ib_year.bounds.y + (ui->ib_year.bounds.height / 2) - 5, 10, 10}, "-");
		GuiTextBox(ui->ib_month.bounds, "0", MAX_INPUT, false);
		GuiLabel((Rectangle){ui->ib_month.bounds.x + ui->ib_month.bounds.width - 1, ui->ib_month.bounds.y + (ui->ib_month.bounds.height / 2) - 5, 10, 10}, "-");
		GuiTextBox(ui->ib_day.bounds, "0", MAX_INPUT, false);
	} else {
		intbox_draw(&ui->ib_year);
		GuiLabel((Rectangle){ui->ib_year.bounds.x + ui->ib_year.bounds.width - 1, ui->ib_year.bounds.y + (ui->ib_year.bounds.height / 2) - 5, 10, 10}, "-");
		intbox_draw(&ui->ib_month);
		GuiLabel((Rectangle){ui->ib_month.bounds.x + ui->ib_month.bounds.width - 1, ui->ib_month.bounds.y + (ui->ib_month.bounds.height / 2) - 5, 10, 10}, "-");
		intbox_draw(&ui->ib_day);
	}

	//textbox_draw(&ui->tb_expiration_date);

	floatbox_draw(&ui->fb_daily_consumption_rate);

	// Panel info
	GuiPanel(ui->panel_bounds, TextFormat("Batch ID retrieved: %s", ui->foodbatch_retrieved.batch_id));

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_back)) {
		*state = STATE_MAIN_MENU;
	}

	if (button_draw_updt(&ui->butn_submit)) {
	}

	if (button_draw_updt(&ui->butn_retrieve)) {
	}

	if (button_draw_updt(&ui->butn_delete)) {
	}

	if (button_draw_updt(&ui->butn_retrieve_all)) {
		db_get_all_food();
	}

	// End button actions

	// Start show warning/error boxes

	// Warnings
	// End show warning/error boxes
}
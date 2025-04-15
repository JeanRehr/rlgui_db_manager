#include <raygui.h>

#include "food_ui.h"
#include "db_manager.h"

typedef struct food_ui_elem food_ui_elem;
typedef enum app_state app_state;
typedef enum error_code error_code;
typedef enum food_screen_flags food_screen_flags;

void draw_food_ui_elem(food_ui_elem *ui, app_state *state, error_code *error)
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
#include <raygui.h>

#include "ui_main_menu.h"

typedef struct ui_main_menu ui_main_menu;
typedef enum app_state app_state;
typedef enum error_code error_code;

void ui_main_menu_init(ui_main_menu *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	ui->reg_resident_butn = button_init((Rectangle){100, 100, 200, 50}, "Manage Persons");

	ui->reg_food_butn = button_init((Rectangle){100, 200, 200, 50}, "Manage Food");
}

void ui_main_menu_draw(ui_main_menu *ui, app_state *state, error_code *error)
{
	// Start draw UI elements
	GuiLabel(ui->menu_title_bounds, "Main Menu");
	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->reg_resident_butn)) {
		*state = STATE_REGISTER_RESIDENT;
	}
	if (button_draw_updt(&ui->reg_food_butn)) {
		*state = STATE_REGISTER_FOOD;
	}
	// End button actions

	// Start show warning/error boxes
	// End show warning/error boxes
}

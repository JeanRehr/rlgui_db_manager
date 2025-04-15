#include <raygui.h>

#include "main_menu_ui.h"

typedef struct main_menu_ui main_menu_ui;
typedef enum app_state app_state;
typedef enum error_code error_code;

void main_menu_ui_init(main_menu_ui *ui)
{
	ui->menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	ui->reg_person_butn = button_init((Rectangle){100, 100, 200, 50}, "Manage Persons");

	ui->reg_food_butn = button_init((Rectangle){100, 200, 200, 50}, "Manage Food");
}

void main_menu_ui_draw(main_menu_ui *ui, app_state *state, error_code *error)
{
	// Start draw UI elements
	GuiLabel(ui->menu_title_bounds, "Main Menu");
	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->reg_person_butn)) {
		*state = STATE_REGISTER_PERSON;
	}
	if (button_draw_updt(&ui->reg_food_butn)) {
		*state = STATE_REGISTER_FOOD;
	}
	// End button actions

	// Start show warning/error boxes
	// End show warning/error boxes
}

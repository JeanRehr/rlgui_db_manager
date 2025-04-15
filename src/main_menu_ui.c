#include <raygui.h>

#include "main_menu_ui.h"

typedef struct main_menu_ui_elem main_menu_ui_elem;
typedef enum app_state app_state;
typedef enum error_code error_code;

void draw_main_menu_ui_elem(main_menu_ui_elem *ui, app_state *state, error_code *error)
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

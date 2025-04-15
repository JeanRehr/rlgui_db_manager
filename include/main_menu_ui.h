#ifndef MAIN_MENU_UI_H
#define MAIN_MENU_UI_H

#include "button.h"
#include "app_state.h"
#include "error_handling.h"

// To manage the state of the main menu screen
struct main_menu_ui_elem {
	Rectangle menu_title_bounds;
	struct button reg_person_butn;
	struct button reg_food_butn;
};

void draw_main_menu_ui_elem(struct main_menu_ui_elem *ui, enum app_state *state, enum error_code *error);

#endif // MAIN_MENU_UI_HF
#ifndef MAIN_MENU_UI_H
#define MAIN_MENU_UI_H

#include "button.h"
#include "app_state.h"
#include "error_handling.h"

// To manage the state of the main menu screen
struct main_menu_ui {
	Rectangle menu_title_bounds;
	struct button reg_person_butn;
	struct button reg_food_butn;
};

void main_menu_ui_draw(struct main_menu_ui *ui, enum app_state *state, enum error_code *error);

#endif // MAIN_MENU_UI_HF
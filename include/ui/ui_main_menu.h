#ifndef UI_MAIN_MENU_H
#define UI_MAIN_MENU_H

#include "app_state.h"
#include "error_handling.h"
#include "ui_elements/button.h"

// To manage the state of the main menu screen
struct ui_main_menu {
    Rectangle menu_title_bounds;
    struct button reg_resident_butn;
    struct button reg_food_butn;
};

void ui_main_menu_init(struct ui_main_menu *ui);

void ui_main_menu_draw(struct ui_main_menu *ui, enum app_state *state, enum error_code *error);

#endif // UI_MAIN_MENU_H
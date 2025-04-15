#ifndef FOOD_UI_H
#define FOOD_UI_H

#include "button.h"
#include "textbox.h"
#include "intbox.h"
#include "floatbox.h"
#include "checkbox.h"
#include "food.h"
#include "app_state.h"
#include "error_handling.h"

enum food_screen_flags {
    FLAG_FOOD_OPERATION_DONE = 1 << 0, // 0001: Submission completed
    FLAG_CONFIRM_FOOD_DELETE = 1 << 1, // 0010: Confirm deletion
};

// To manage the state of the register food screen
struct food_ui_elem {
	Rectangle menu_title_bounds;
	struct intbox ib_batch_id;

	struct textbox tb_name;
	struct intbox ib_quantity;

	struct checkbox cb_is_perishable;

	struct intbox ib_year;
	struct intbox ib_month;
	struct intbox ib_day;

	struct floatbox fb_daily_consumption_rate;

	struct button butn_back;
	struct button butn_submit;
	struct button butn_retrieve;
	struct button butn_delete;
	struct button butn_retrieve_all;

	Rectangle panel_bounds;
	struct foodbatch foodbatch_retrieved;

    enum food_screen_flags flag;
};

void draw_food_ui_elem(struct food_ui_elem *ui, enum app_state *state, enum error_code *error);

#endif // FOOD_UI_H
#ifndef UI_FOOD_H
#define UI_FOOD_H

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
	// batch_id will never be empty actually
	//FLAG_ID_EMPTY = 1 << 2, // 0100: Unique Identifier of batch id empty
	FLAG_BATCHID_EXISTS = 1 << 2, // 0100: Unique Identifier already exists in database
	FLAG_INVALID_FOOD_DATE = 1 << 3, // 1000: Invalid date entered
	FLAG_BATCHID_NOT_FOUND = 1 << 4, // 10000: batch_id not found in the database
};

// To manage the state of the register food screen
struct ui_food {
	Rectangle menu_title_bounds;
	struct intbox ib_batch_id;

	struct textbox tb_name;
	struct intbox ib_quantity;

	struct checkbox cb_is_perishable;

	Rectangle expirationDateText;

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

void ui_food_init(struct ui_food *ui);

void ui_food_draw(struct ui_food *ui, enum app_state *state, enum error_code *error);

void ui_food_updt_pos(struct ui_food *ui);

#endif // UI_FOOD_H
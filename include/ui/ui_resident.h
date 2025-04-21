#ifndef UI_RESIDENT_H
#define UI_RESIDENT_H

#include "db/db_manager.h"
#include "ui_elements/button.h"
#include "ui_elements/textbox.h"
#include "ui_elements/intbox.h"
#include "ui_elements/dropdownbox.h"
#include "ui_elements/checkbox.h"
#include "resident.h"
#include "app_state.h"
#include "error_handling.h"

// Flags to manage the popups, maybe this is specific only to resident screen, so should be an attr of the resident screen struct?
enum resident_screen_flags {
	FLAG_RESIDENT_OPERATION_DONE = 1 << 0, // 0001: Submission completed
	FLAG_CONFIRM_RESIDENT_DELETE = 1 << 1, // 0010: Confirm deletion
	FLAG_CPF_EXISTS = 1 << 2, // 0100: CPF already exists in database
	FLAG_CPF_NOT_FOUND = 1 << 3, // 1000: CPF was not found in database
	FLAG_INPUT_CPF_EMPTY = 1 << 4, // 10000: Input form textbox CPF is empty
	FLAG_CPF_NOT_VALID = 1 << 5, // 100000: CPF input is not valid
	FLAG_SHOW_HEALTH = 1 << 6, // 1000000: Show Health popup
	FLAG_SHOW_NEEDS = 1 << 7, // 10000000: Show Needs popup
};

// To manage the state of the register resident screen
struct ui_resident {
	Rectangle menu_title_bounds;
	struct textbox tb_name;
	struct textbox tb_cpf; // cpf makes sense still being a textbox, as it needs to contain zeroes at the start

	struct intbox ib_age;
	
	struct textbox tb_health_status;
	struct textbox tb_needs;

	struct checkbox cb_medical_assistance;

	struct dropdownbox ddb_gender;

	struct button butn_back;
	struct button butn_submit;
	struct button butn_retrieve;
	struct button butn_delete;
	struct button butn_retrieve_all;

	Rectangle panel_bounds;
	struct resident resident_retrieved;

    enum resident_screen_flags flag;
};

void ui_resident_init(struct ui_resident *ui);

void ui_resident_draw(struct ui_resident *ui, enum app_state *state, enum error_code *error, database *resident_db);

void ui_resident_updt_pos(struct ui_resident *ui);

#endif // UI_RESIDENT_H
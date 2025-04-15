#ifndef PERSON_UI_H
#define PERSON_UI_H

#include "button.h"
#include "textbox.h"
#include "intbox.h"
#include "dropdownbox.h"
#include "person.h"
#include "app_state.h"
#include "error_handling.h"

// Flags to manage the popups, maybe this is specific only to person screen, so should be an attr of the person screen struct?
enum person_screen_flags {
	FLAG_NONE = 0, // No flags set
	FLAG_CONFIRM_DELETE = 1 << 0, // 0001: Confirm deletion
	FLAG_OPERATION_DONE = 1 << 1, // 0010: Submission completed
	FLAG_CPF_EXISTS = 1 << 2, // 0100: CPF already exists in database
	FLAG_CPF_NOT_FOUND = 1 << 3, // 1000: CPF was not found in database
	FLAG_INPUT_CPF_EMPTY = 1 << 4, // 10000: Input form textbox CPF is empty
	FLAG_CPF_NOT_VALID = 1 << 5, // 100000: CPF input is not valid
	FLAG_SHOW_HEALTH = 1 << 6, // 1000000: Show Health popup
	FLAG_SHOW_NEEDS = 1 << 7, // 10000000: Show Needs popup
};

// To manage the state of the register person screen
struct person_ui_elem {
	Rectangle menu_title_bounds;
	struct textbox tb_name;
	struct textbox tb_cpf; // cpf makes sense still being a textbox, as it needs to contain zeroes at the start

	struct intbox ib_age;
	
	struct textbox tb_health_status;
	struct textbox tb_needs;

	struct dropdownbox ddb_gender;

	struct button butn_back;
	struct button butn_submit;
	struct button butn_retrieve;
	struct button butn_delete;
	struct button butn_retrieve_all;

	Rectangle panel_bounds;
	struct person person_retrieved;

    enum person_screen_flags flag;
};

void draw_person_ui_elem(struct person_ui_elem *ui, enum app_state *state, enum error_code *error);

#endif // PERSON_UI_H
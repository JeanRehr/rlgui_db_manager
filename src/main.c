/*
 * how many people there are in total inside the database
 * calculate the average of food that 1 person needs
 * how many babies (for baby specific stuff) and woman (for woman specific stuff)
 * if anyone needs any pharmacies
 * total food that is in stock and total that needs to be maintained for 1 month
 * calculate for how long the current food will last
 * how long will take for the food to spoil in case of milk, eggs...
 * register people that enters
 * register food that enters
 * register people that gets out
 * register food that gets out
 * two databases, one for food and one for people
 * one screen for inserting food, one screen for inserting people
 */

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// raygui embedded styles
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE   12       // NOTE: Included light style
#include "styles/style_jungle.h"            // raygui style: jungle
#include "styles/style_candy.h"             // raygui style: candy
#include "styles/style_lavanda.h"           // raygui style: lavanda
#include "styles/style_cyber.h"             // raygui style: cyber
#include "styles/style_terminal.h"          // raygui style: terminal
#include "styles/style_ashes.h"             // raygui style: ashes
#include "styles/style_bluish.h"            // raygui style: bluish
#include "styles/style_dark.h"              // raygui style: dark
#include "styles/style_cherry.h"            // raygui style: cherry
#include "styles/style_sunny.h"             // raygui style: sunny
#include "styles/style_enefete.h"           // raygui style: enefete

#include <stdio.h>
#include <string.h>

#include "CONSTANTS.h"
#include "utilsfn.h"

#include "db_manager.h"
#include "person.h"
#include "textbox.h"
#include "dropdownbox.h"
#include "button.h"

int TOTAL_PERSONS; // Total number of persons in the database, used for tracking

// typedefs
typedef struct textbox textbox;
typedef struct dropdownbox dropdownbox;
typedef struct button button;
typedef struct person person;

// FOOD

typedef struct food_item {
	char name[MAX_INPUT];
	int quantity;
	char expiration_date[11]; // ISO 8601 format 2000-12-31
} food_item;

// END FOOD

// To manage the state of the main menu screen
typedef struct main_menu_ui_elemnts {
	Rectangle menu_title_bounds;
	button reg_person_butn;
	button reg_food_Butn;
} main_menu_ui_elemnts;

// To manage the state of the register person screen
typedef struct register_person_ui_elemnts {
	Rectangle menu_title_bounds;
	textbox tb_name;
	textbox tb_cpf;
	textbox tb_age;
	textbox tb_health_status;
	textbox tb_needs;

	dropdownbox ddb_gender;

	button butn_back;
	button butn_submit;
	button butn_retrieve;
	button butn_delete;
	button butn_retrieve_all;

	Rectangle panel_bounds;	
	person person_retrieved;
} register_person_ui_elemnts;

// To manage the state of the register food screen

typedef enum flags_popup {
	FLAG_NONE = 0, // No flags set
	FLAG_SHOW_HEALTH = 1 << 0, // 0001: Show Health popup
	FLAG_SHOW_NEEDS = 1 << 1, // 0010: Show Needs popup
	FLAG_CONFIRM_DELETE = 1 << 2, // 0100: Confirm deletion
	FLAG_OPERATION_DONE = 1 << 3, // 1000: Submission completed
	FLAG_CPF_EXISTS = 1 << 4,
	FLAG_CPF_NOT_FOUND = 1 << 5,
	FLAG_INPUT_CPF_EMPTY = 1 << 6,
	FLAG_CPF_NOT_VALID = 1 << 7,
} flags_popup;

typedef enum error_code {
	NO_ERROR = 0,
	ERROR_DATABASE,
} error_code;

typedef enum app_state {
	STATE_MAIN_MENU = 0,
	STATE_REGISTER_PERSON,
	STATE_REGISTER_FOOD,
} app_state;

int window_width = 1600;
int window_height = 800;

int active_style = 8; // Set default style to dark
int prev_active_style = 7;

// Utility functions for flags
static inline void set_flag(flags_popup *flags, flags_popup flag);

static inline void clear_flag(flags_popup *flags, flags_popup flag);

static inline bool is_flag_set(flags_popup *flags, flags_popup flag);

void draw_main_menu_screen(main_menu_ui_elemnts *ui, app_state *state, error_code *error);

void draw_register_person_screen(register_person_ui_elemnts *ui, app_state *state, error_code *error, flags_popup *flags);

int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(window_width, window_height, "Shelter Management");
	
	// Initialize databases, close window and program if database initialization fails
	if (db_init() != SQLITE_OK) {
		CloseWindow(); 
		return -1;
	}

	GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

	SetTargetFPS(60);

	// Start initializing the register food screen

	main_menu_ui_elemnts main_menu_screen;
	
	main_menu_screen.menu_title_bounds = (Rectangle) {20, 20, 120, 30};
	main_menu_screen.reg_person_butn = button_init((Rectangle){20, main_menu_screen.menu_title_bounds.y + main_menu_screen.menu_title_bounds.height, 100, 30}, "Register Person");
	main_menu_screen.reg_food_Butn = button_init((Rectangle){main_menu_screen.reg_person_butn.bounds.x + main_menu_screen.reg_person_butn.bounds.width + 10, main_menu_screen.reg_person_butn.bounds.y, 100, 30}, "Register Food");

	// End initializing the register food screen

	// Start initializing the register person screen in order as they appear from top-down top-left

	register_person_ui_elemnts register_person_screen;

	register_person_screen.menu_title_bounds = (Rectangle) {20, 20, 200, 16};

	register_person_screen.butn_back = button_init((Rectangle) {20, register_person_screen.menu_title_bounds.y + (register_person_screen.menu_title_bounds.height * 2), 0, 30}, "Back");
	register_person_screen.tb_name = textbox_init(
		(Rectangle){20, register_person_screen.butn_back.bounds.y + (register_person_screen.butn_back.bounds.height * 2), 300, 30},
		"Name:",
		INPUT_TEXT,
		0
	);
	register_person_screen.tb_cpf = textbox_init(
		(Rectangle){20, register_person_screen.tb_name.bounds.y + (register_person_screen.tb_name.bounds.height * 2), 300, 30},
		"CPF:",
		INPUT_INTEGER,
		11
	);
	register_person_screen.tb_age = textbox_init(
		(Rectangle){20, register_person_screen.tb_cpf.bounds.y + (register_person_screen.tb_cpf.bounds.height * 2), 300, 30},
		"Age:",
		INPUT_INTEGER,
		3
	);
	register_person_screen.tb_health_status = textbox_init(
		(Rectangle){20, register_person_screen.tb_age.bounds.y + (register_person_screen.tb_age.bounds.height * 2), 300, 30},
		"Health Status:",
		INPUT_TEXT,
		0
	);
	register_person_screen.tb_needs = textbox_init(
		(Rectangle){20, register_person_screen.tb_health_status.bounds.y + (register_person_screen.tb_health_status.bounds.height * 2), 300, 30}, 
		"Needs:",
		INPUT_TEXT,
		0
	);

	register_person_screen.ddb_gender = dropdownbox_init(
		(Rectangle){20, register_person_screen.tb_needs.bounds.y + (register_person_screen.tb_needs.bounds.height * 2), 200, 30},
		"Other;Male;Female",
		"Gender"
	);

	register_person_screen.butn_submit = button_init((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	register_person_screen.butn_retrieve = button_init((Rectangle) {register_person_screen.butn_submit.bounds.x + register_person_screen.butn_submit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	register_person_screen.butn_delete = button_init((Rectangle) {register_person_screen.butn_retrieve.bounds.x + register_person_screen.butn_retrieve.bounds.width + 10, window_height - 100, 100, 30}, "Delete");
	register_person_screen.butn_retrieve_all = button_init((Rectangle) {register_person_screen.butn_delete.bounds.x + register_person_screen.butn_delete.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve All");

	memset(&register_person_screen.person_retrieved, 0, sizeof(person));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register person screen function
	register_person_screen.panel_bounds = (Rectangle) {window_width / 2 - 200, 10, 275, 200};

	// End initializing the register person screen

	// Rectangle bounds for the style selector box, it is persistent across all screens
	Rectangle style_options_bounds = {window_width - 130, 30, 120, 30};
	Rectangle style_options_label = {style_options_bounds.x, style_options_bounds.y - 25, style_options_bounds.width, 20};

	// Setting the initial state for screen and error code
	error_code error = NO_ERROR;
	app_state app_state = STATE_MAIN_MENU;
	flags_popup flags_popup = FLAG_NONE;

	while (!WindowShouldClose()) {
		// Update
		//----------------------------------------------------------------------------------
		GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

		if (IsWindowResized()) {
			window_width = GetScreenWidth();
			window_height = GetScreenHeight();
			register_person_screen.butn_submit.bounds.y = window_height - 100;
			register_person_screen.butn_retrieve.bounds.y = window_height - 100;
			register_person_screen.butn_delete.bounds.y = window_height - 100;
			register_person_screen.butn_retrieve_all.bounds.y = window_height - 100;
			register_person_screen.panel_bounds.x = window_width / 2 - 200;
			style_options_bounds.x = window_width - 130;
			style_options_label.x = style_options_bounds.x;
		}

		if (active_style != prev_active_style) {
			// Reset to default internal style
			// NOTE: Required to unload any previously loaded font texture
			GuiLoadStyleDefault();

			switch (active_style) {
				case 1: GuiLoadStyleJungle(); break;
				case 2: GuiLoadStyleCandy(); break;
				case 3: GuiLoadStyleLavanda(); break;
				case 4: GuiLoadStyleCyber(); break;
				case 5: GuiLoadStyleTerminal(); break;
				case 6: GuiLoadStyleAshes(); break;
				case 7: GuiLoadStyleBluish(); break;
				case 8: GuiLoadStyleDark(); break;
				case 9: GuiLoadStyleCherry(); break;
				case 10: GuiLoadStyleSunny(); break;
				case 11: GuiLoadStyleEnefete(); break;
				default: break;
			}

			prev_active_style = active_style;
		}
		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		// Visuals options
		GuiLabel(style_options_label, "Style:");
		GuiComboBox(style_options_bounds, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &active_style);

		switch (app_state) {
		case STATE_MAIN_MENU:
			draw_main_menu_screen(&main_menu_screen, &app_state, &error);
			break;
		
		case STATE_REGISTER_PERSON:
			draw_register_person_screen(&register_person_screen, &app_state, &error, &flags_popup);
			break;
		
		case STATE_REGISTER_FOOD:
			break;

		default:
			break;
		}

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();
	//--------------------------------------------------------------------------------------
	return 0;
}

// Utility functions for flags
static inline void set_flag(flags_popup *flags, flags_popup flag) {
	*flags |= flag;
}

static inline void clear_flag(flags_popup *flags, flags_popup flag) {
	*flags &= ~flag;
}

static inline bool is_flag_set(flags_popup *flags, flags_popup flag) {
	return (*flags & flag) != 0;
}

void draw_main_menu_screen(main_menu_ui_elemnts *ui, app_state *state, error_code *error)
{
	// Start draw UI elements
	GuiLabel(ui->menu_title_bounds, "Main Menu");
	// End draw UI elements

	// Start button actions
	if (button_draw_updt(&ui->reg_person_butn)) {
		*state = STATE_REGISTER_PERSON;
	}

	if (button_draw_updt(&ui->reg_food_Butn)) {
		// Screen not yet implemented
	}
	// End button actions

	// Start show warning/error boxes
	// End show warning/error boxes
}

void draw_register_person_screen(register_person_ui_elemnts *ui, app_state *state, error_code *error, flags_popup *flag)
{
	// Start draw UI elements

	GuiLabel(ui->menu_title_bounds, "Register Person menu");

	textbox_draw(&ui->tb_name);
	textbox_draw(&ui->tb_cpf);
	textbox_draw(&ui->tb_age);

	dropdownbox_draw(&ui->ddb_gender);

	textbox_draw(&ui->tb_health_status);
	textbox_draw(&ui->tb_needs);

	// Info Panel
	GuiPanel(ui->panel_bounds, TextFormat("CPF info retrieved: %s", ui->person_retrieved.cpf));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 30, 280, 20}, TextFormat("Name: %s", ui->person_retrieved.name));
	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 60, 280, 20}, TextFormat("Age: %d", ui->person_retrieved.age));

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 90, 280, 20}, TextFormat("Health Status: %.15s...", ui->person_retrieved.health_status));
	if (GuiButton((Rectangle){ui->panel_bounds.x + 10 + 235, ui->panel_bounds.y + 90, 20, 20}, "?")) {
		set_flag(flag, FLAG_SHOW_HEALTH);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 120, 280, 20}, TextFormat("Needs: %.15s...", ui->person_retrieved.needs));
	if (GuiButton((Rectangle){ui->panel_bounds.x + 10 + 235, ui->panel_bounds.y + 120, 20, 20}, "?")) {
		set_flag(flag, FLAG_SHOW_NEEDS);
	}

	GuiLabel((Rectangle){ui->panel_bounds.x + 10, ui->panel_bounds.y + 150, 280, 20}, TextFormat("Gender: %s", ui->person_retrieved.gender == GENDER_OTHER ? "Other" : ui->person_retrieved.gender == GENDER_MALE ? "Male" : "Female"));
	
	if (is_flag_set(flag, FLAG_SHOW_HEALTH)) {
		char wrapped_text[MAX_INPUT + 16] = {0}; // +16 to prevent overflow from adding '\n'
		wrap_text(ui->person_retrieved.health_status, wrapped_text, 35);
		int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 300},
								   "#191#Full Health Status", wrapped_text, "Close");
		if (result >= 0) clear_flag(flag, FLAG_SHOW_HEALTH);
	}

	if (is_flag_set(flag, FLAG_SHOW_NEEDS)) {
		char wrapped_text[MAX_INPUT + 16] = {0}; // +16 to prevent overflow from adding '\n'
		wrap_text(ui->person_retrieved.needs, wrapped_text, 35);
		int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2, 300, 300},
								   "#191#Full Needs", wrapped_text, "Close");
		if (result >= 0) clear_flag(flag, FLAG_SHOW_NEEDS);
	}

	// End draw UI elements

	// Start button actions

	if (button_draw_updt(&ui->butn_back)) {
		*state = STATE_MAIN_MENU;
	}

	if (button_draw_updt(&ui->butn_submit)) {
		if (*ui->tb_cpf.input == '\0') {
			set_flag(flag, FLAG_INPUT_CPF_EMPTY);
			fprintf(stderr, "CPF must not be empty.\n");
		} else if (!is_valid_integer_input(ui->tb_cpf.input, 11, 11)) {
			set_flag(flag, FLAG_CPF_NOT_VALID);
			fprintf(stderr, "CPF must be 11 digits.\n");
		} else if (db_check_cpf_exists(ui->tb_cpf.input)) {
			set_flag(flag, FLAG_CPF_EXISTS);
		} else if (db_insert_person(ui->tb_cpf.input, ui->tb_name.input, atoi(ui->tb_age.input), ui->tb_health_status.input, ui->tb_needs.input, ui->ddb_gender.active_option) != SQLITE_OK) {
			*error = ERROR_DATABASE;
			fprintf(stderr, "Error submitting to database.\n");
		} else {
			*error = NO_ERROR;
			set_flag(flag, FLAG_OPERATION_DONE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve)) {
		if (db_get_person_by_cpf(ui->tb_cpf.input, &ui->person_retrieved)) {
			printf("Retrieved Person - Name: %s, Age: %d, Health Status: %s, Needs: %s, Gender: %d\n", ui->person_retrieved.name, ui->person_retrieved.age, ui->person_retrieved.health_status, ui->person_retrieved.needs, ui->person_retrieved.gender);
			set_flag(flag, FLAG_OPERATION_DONE);
		} else {
			set_flag(flag, FLAG_CPF_NOT_FOUND);
		}
	}

	if (button_draw_updt(&ui->butn_delete)) {
		if (*ui->tb_cpf.input == '\0') {
			set_flag(flag, FLAG_INPUT_CPF_EMPTY);
		} else if (!is_valid_integer_input(ui->tb_cpf.input, 11, 11)) {
			set_flag(flag, FLAG_CPF_NOT_VALID);
		} else if (!db_check_cpf_exists(ui->tb_cpf.input)) {
			set_flag(flag, FLAG_CPF_NOT_FOUND);
		} else {
			set_flag(flag, FLAG_CONFIRM_DELETE);
		}
	}

	if (button_draw_updt(&ui->butn_retrieve_all)) {
		db_get_all_persons();
	}

	// End button actions

	// Start show warning/error boxes

	// In case updating person
	if (is_flag_set(flag, FLAG_CPF_EXISTS)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF Already exists.", "Update;Don't update");
		if (result == 1) {
			if (db_update_person(ui->tb_cpf.input, ui->tb_name.input, atoi(ui->tb_age.input), ui->tb_health_status.input, ui->tb_needs.input, ui->ddb_gender.active_option) != SQLITE_OK) {
				*error = ERROR_DATABASE;
			}
		}
		if (result >= 0) {
			*error = NO_ERROR;
			clear_flag(flag, FLAG_CPF_EXISTS);
		}
	}

	// In case deleting person
	if (is_flag_set(flag, FLAG_CONFIRM_DELETE)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Deleting Person!", "Are you sure you want to delete?", "Yes, delete;NO");
		if (result == 1) {
			db_delete_person_by_cpf(ui->tb_cpf.input);
			set_flag(flag, FLAG_OPERATION_DONE);
		}
		if (result >= 0) {
			clear_flag(flag, FLAG_CONFIRM_DELETE);
		}
	}

	if (is_flag_set(flag, FLAG_INPUT_CPF_EMPTY)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must not be empty.", "OK");
		if (result >= 0) {
			clear_flag(flag, FLAG_INPUT_CPF_EMPTY);
		}
	} else if (is_flag_set(flag, FLAG_CPF_NOT_VALID)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must be 11 digits.", "OK");
		if (result >= 0) {
			clear_flag(flag, FLAG_CPF_NOT_VALID);
		}
	} else if (is_flag_set(flag, FLAG_CPF_NOT_FOUND)) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF not found.", "OK");
		if (result >= 0) {
			clear_flag(flag, FLAG_CPF_NOT_FOUND);
		}
	} else if (*error == ERROR_DATABASE) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Error submitting to database.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	}

	// End show warning/error boxes

	// Clear the text buffer only after a successfull operation
	if (is_flag_set(flag, FLAG_OPERATION_DONE)) {
		ui->tb_name.input[0] = '\0';
		ui->tb_cpf.input[0] = '\0';
		ui->tb_age.input[0] = '\0';
		ui->tb_health_status.input[0] = '\0';
		ui->tb_needs.input[0] = '\0';
		clear_flag(flag, FLAG_OPERATION_DONE);
	}
}
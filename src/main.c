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
#define MAX_GUI_STYLES_AVAILABLE   13       // NOTE: Included light style
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
#include "styles/style_amber.h"

#include <stdio.h>
#include <string.h>

#include "CONSTANTS.h"
#include "utilsfn.h"
#include "globals.h"

#include "db_manager.h"
#include "person.h"
#include "textbox.h"
#include "intbox.h"
#include "floatbox.h"
#include "checkbox.h"
#include "dropdownbox.h"
#include "button.h"
#include "food.h"
#include "main_menu_ui.h"
#include "person_ui.h"
#include "food_ui.h"
#include "error_handling.h"
#include "app_state.h"

int TOTAL_PERSONS; // Total number of persons in the database, used for tracking

// typedefs
typedef struct textbox textbox;
typedef struct intbox intbox;
typedef struct floatbox floatbox;
typedef struct checkbox checkbox;
typedef struct dropdownbox dropdownbox;
typedef struct button button;
typedef struct person person;
typedef struct foodbatch foodbatch;
typedef struct main_menu_ui_elem main_menu_ui_elem;
typedef struct person_ui_elem person_ui_elem;
typedef struct food_ui_elem food_ui_elem;

typedef enum error_code error_code;
typedef enum app_state app_state;

int active_style = 8; // Set default style to dark
int prev_active_style = 7;

int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(window_width, window_height, "Shelter Management");
	
	// Initialize databases, close window and program if database initialization fails
	if (db_init() != SQLITE_OK) {
		CloseWindow(); 
		return ERROR_OPENING_DB;
	}

	GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

	SetTargetFPS(60);

	// Start initializing the main menu screen in order as they appear from top-down top-left

	main_menu_ui_elem main_menu_ui_elem;
	
	main_menu_ui_elem.menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	main_menu_ui_elem.reg_person_butn = button_init((Rectangle){100, 100, 200, 50}, "Manage Persons");

	main_menu_ui_elem.reg_food_butn = button_init((Rectangle){100, 200, 200, 50}, "Manage Food");

	// End initializing the main menu screen

	// Start initializing the register person screen in order as they appear from top-down top-left

	person_ui_elem person_ui_elem;

	person_ui_elem.menu_title_bounds = (Rectangle) {10, 10, 120, 20};

	person_ui_elem.butn_back = button_init(
		(Rectangle) {20, person_ui_elem.menu_title_bounds.y + (person_ui_elem.menu_title_bounds.height * 2), 0, 30},
		"Back"
	);
	person_ui_elem.tb_name = textbox_init(
		(Rectangle){20, person_ui_elem.butn_back.bounds.y + (person_ui_elem.butn_back.bounds.height * 2), 300, 30},
		"Name:",
		INPUT_TEXT,
		0
	);
	person_ui_elem.tb_cpf = textbox_init(
		(Rectangle){20, person_ui_elem.tb_name.bounds.y + (person_ui_elem.tb_name.bounds.height * 2), 300, 30},
		"CPF:",
		INPUT_INTEGER,
		11
	);
	person_ui_elem.ib_age = intbox_init(
		(Rectangle){20, person_ui_elem.tb_cpf.bounds.y + (person_ui_elem.tb_cpf.bounds.height * 2), 125, 30},
		"Age:",
		0,
		120
	);
	person_ui_elem.tb_health_status = textbox_init(
		(Rectangle){20, person_ui_elem.ib_age.bounds.y + (person_ui_elem.ib_age.bounds.height * 2), 300, 30},
		"Health Status:",
		INPUT_TEXT,
		0
	);
	person_ui_elem.tb_needs = textbox_init(
		(Rectangle){20, person_ui_elem.tb_health_status.bounds.y + (person_ui_elem.tb_health_status.bounds.height * 2), 300, 30}, 
		"Needs:",
		INPUT_TEXT,
		0
	);

	person_ui_elem.ddb_gender = dropdownbox_init(
		(Rectangle){20, person_ui_elem.tb_needs.bounds.y + (person_ui_elem.tb_needs.bounds.height * 2), 200, 30},
		"Other;Male;Female",
		"Gender"
	);

	person_ui_elem.butn_submit = button_init((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	person_ui_elem.butn_retrieve = button_init((Rectangle) {person_ui_elem.butn_submit.bounds.x + person_ui_elem.butn_submit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	person_ui_elem.butn_delete = button_init((Rectangle) {person_ui_elem.butn_retrieve.bounds.x + person_ui_elem.butn_retrieve.bounds.width + 10, window_height - 100, 100, 30}, "Delete");
	person_ui_elem.butn_retrieve_all = button_init((Rectangle) {person_ui_elem.butn_delete.bounds.x + person_ui_elem.butn_delete.bounds.width + 10, window_height - 100, 0, 30}, "Retrieve All");

	memset(&person_ui_elem.person_retrieved, 0, sizeof(person));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register person screen function
	person_ui_elem.panel_bounds = (Rectangle) {window_width / 2 - 200, 10, 300, 200};

	person_ui_elem.flag = 0;

	// End initializing the register person screen

	// Start initializing the register food screen in order as they appear from top-down top-left

	food_ui_elem food_ui_elem;

	food_ui_elem.menu_title_bounds = (Rectangle) {10, 10, 150, 20};

	food_ui_elem.butn_back = button_init(
		(Rectangle) {20, food_ui_elem.menu_title_bounds.y + (food_ui_elem.menu_title_bounds.height * 2), 0, 30},
		"Back"
	);

	food_ui_elem.ib_batch_id = intbox_init(
		(Rectangle){20, food_ui_elem.butn_back.bounds.y + (food_ui_elem.butn_back.bounds.height * 2), 130, 30},
		"Batch ID:",
		1,
		99999999
	);
	food_ui_elem.tb_name = textbox_init(
		(Rectangle){20, food_ui_elem.ib_batch_id.bounds.y + (food_ui_elem.ib_batch_id.bounds.height * 2), 300, 30},
		"Food Name:",
		INPUT_TEXT,
		0
	);
	food_ui_elem.ib_quantity = intbox_init(
		(Rectangle){20, food_ui_elem.tb_name.bounds.y + (food_ui_elem.tb_name.bounds.height * 2), 125, 30},
		"Quantity:",
		0,
		INT_MAX
	);

	food_ui_elem.cb_is_perishable = checkbox_init(
		(Rectangle){20, food_ui_elem.ib_quantity.bounds.y + (food_ui_elem.ib_quantity.bounds.height * 2), 20, 20}, 
		"Is Perishable?"
	);


	food_ui_elem.ib_year = intbox_init(
		(Rectangle) {20, food_ui_elem.cb_is_perishable.bounds.y + (food_ui_elem.cb_is_perishable.bounds.height * 2), 40, 30}, "Year", 1000, 2025
	);

	food_ui_elem.ib_month = intbox_init(
		(Rectangle) {food_ui_elem.ib_year.bounds.x + food_ui_elem.ib_year.bounds.width + 5, food_ui_elem.cb_is_perishable.bounds.y + (food_ui_elem.cb_is_perishable.bounds.height * 2), 35, 30}, "Month", 1, 12
	);

	food_ui_elem.ib_day = intbox_init(
		(Rectangle) {food_ui_elem.ib_month.bounds.x + food_ui_elem.ib_month.bounds.width + 5, food_ui_elem.cb_is_perishable.bounds.y + (food_ui_elem.cb_is_perishable.bounds.height * 2), 35, 30}, "Day", 1, 31
	);

	food_ui_elem.fb_daily_consumption_rate = floatbox_init(
		(Rectangle){20, food_ui_elem.ib_year.bounds.y + (food_ui_elem.ib_year.bounds.height * 2), 250, 30},
		"Avg daily consumption rate per person?"
	);

	food_ui_elem.butn_submit = button_init((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	food_ui_elem.butn_retrieve = button_init((Rectangle) {food_ui_elem.butn_submit.bounds.x + food_ui_elem.butn_submit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	food_ui_elem.butn_delete = button_init((Rectangle) {food_ui_elem.butn_retrieve.bounds.x + food_ui_elem.butn_retrieve.bounds.width + 10, window_height - 100, 100, 30}, "Delete");
	food_ui_elem.butn_retrieve_all = button_init((Rectangle) {food_ui_elem.butn_delete.bounds.x + food_ui_elem.butn_delete.bounds.width + 10, window_height - 100, 0, 30}, "Retrieve All");

	memset(&food_ui_elem.foodbatch_retrieved, 0, sizeof(foodbatch));
	
	// Only set the bounds of the panel, draw everything inside based on it on the draw register person screen function
	food_ui_elem.panel_bounds = (Rectangle) {window_width / 2 - 200, 10, 300, 200};

	food_ui_elem.flag = 0;

	// End initializing the register food screen

	// Rectangle bounds for the style selector box, it is persistent across all screens
	Rectangle style_options_bounds = {window_width - 130, 30, 120, 30};
	Rectangle style_options_label = {style_options_bounds.x, style_options_bounds.y - 25, style_options_bounds.width, 20};

	// Setting the initial state for screen and error code
	error_code error = NO_ERROR;
	app_state app_state = STATE_MAIN_MENU;

	while (!WindowShouldClose()) {
		// Update
		//----------------------------------------------------------------------------------
		GuiSetStyle(DEFAULT, TEXT_SIZE, FONT_SIZE);

		if (IsWindowResized()) {
			update_window_size(GetScreenWidth(), GetScreenHeight());
			person_ui_elem.butn_submit.bounds.y = window_height - 100;
			person_ui_elem.butn_retrieve.bounds.y = window_height - 100;
			person_ui_elem.butn_delete.bounds.y = window_height - 100;
			person_ui_elem.butn_retrieve_all.bounds.y = window_height - 100;
			person_ui_elem.panel_bounds.x = window_width / 2 - 200;
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
				case 12: GuiLoadStyleAmber(); break;
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
		GuiComboBox(style_options_bounds, "Default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete;Amber", &active_style);

		switch (app_state) {
		case STATE_MAIN_MENU:
			draw_main_menu_ui_elem(&main_menu_ui_elem, &app_state, &error);
			break;
		
		case STATE_REGISTER_PERSON:
			draw_person_ui_elem(&person_ui_elem, &app_state, &error);
			break;
		
		case STATE_REGISTER_FOOD:
			draw_food_ui_elem(&food_ui_elem, &app_state, &error);
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
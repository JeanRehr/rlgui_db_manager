/*
 * how many people there are in total inside the database
 * calculate the average of food that 1 resident needs
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

#include "CONSTANTS.h"
#include "utilsfn.h"
#include "globals.h"

#include "db_manager.h"
#include "resident.h"
#include "textbox.h"
#include "intbox.h"
#include "floatbox.h"
#include "checkbox.h"
#include "dropdownbox.h"
#include "button.h"
#include "food.h"
#include "ui_main_menu.h"
#include "ui_resident.h"
#include "ui_food.h"
#include "error_handling.h"
#include "app_state.h"

// typedefs
typedef struct textbox textbox;
typedef struct intbox intbox;
typedef struct floatbox floatbox;
typedef struct checkbox checkbox;
typedef struct dropdownbox dropdownbox;
typedef struct button button;
typedef struct resident resident;
typedef struct foodbatch foodbatch;
typedef struct ui_main_menu ui_main_menu;
typedef struct ui_resident ui_resident;
typedef struct ui_food ui_food;

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

	ui_main_menu ui_main_menu;

	ui_main_menu_init(&ui_main_menu);

	// End initializing the main menu screen

	// Start initializing the register resident screen in order as they appear from top-down top-left

	ui_resident ui_resident;

	ui_resident_init(&ui_resident);

	// End initializing the register resident screen

	// Start initializing the register food screen in order as they appear from top-down top-left

	ui_food ui_food;

	ui_food_init(&ui_food);

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
			ui_resident_updt_pos(&ui_resident);
			ui_food_updt_pos(&ui_food);
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
		case STATE_LOGIN_MENU:
			//login_menu_draw();
			break;
		case STATE_MAIN_MENU:
			ui_main_menu_draw(&ui_main_menu, &app_state, &error);
			break;
		
		case STATE_REGISTER_PERSON:
			ui_resident_draw(&ui_resident, &app_state, &error);
			break;
		
		case STATE_REGISTER_FOOD:
			ui_food_draw(&ui_food, &app_state, &error);
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
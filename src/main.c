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
#include "Person.h"
#include "Textbox.h"
#include "Dropdownbox.h"
#include "Button.h"

int TOTAL_PERSONS; // Total number of persons in the database, used for tracking

// typedefs
typedef struct Textbox Textbox;
typedef struct Dropdownbox Dropdownbox;
typedef struct Button Button;
typedef struct Person Person;

// FOOD

typedef struct FoodItem {
	char name[MAX_INPUT];
	int quantity;
	char expirationDate[11]; // ISO 8601 format 2000-12-31
} FoodItem;

// END FOOD

// To manage the state of the insert person screen
typedef struct INSERT_PERSON_UIElements {
	Textbox tbName;
	Textbox tbCpf;
	Textbox tbAge;
	Textbox tbHealthStatus;
	Textbox tbNeeds;

	Dropdownbox ddbGender;

	Button buttonBack;
	Button buttonSubmit;
	Button buttonRetrieve;

	bool showHealthPopup;
	bool showNeedsPopup;
	Rectangle panelBounds;	
	Person personRetrieved;
} INSERT_PERSON_UIElements;

typedef enum ErrorCode {
	NIL = 0,
	NO_ERROR,
	ERROR_DATABASE,
	ERROR_CPF_NOT_VALID,
	ERROR_CPF_EMPTY,
	ERROR_CPF_ALERADY_EXISTS,
	ERROR_CPF_NOT_FOUND,
} ErrorCode;

typedef enum AppState {
	STATE_MAIN_MENU = 0,
	STATE_INSERT_PERSON,
	STATE_INSERT_FOOD,
} AppState;

int window_width = 1600;
int window_height = 800;

int visualStyleActive = 0;
int prevVisualStyleActive = 0;

void DrawInsertPersonScreen(INSERT_PERSON_UIElements *ui, AppState *state, ErrorCode *error);

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

	GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
	int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);

	SetTargetFPS(60);

	// Start initializing the insert person screen in order as they appear

	INSERT_PERSON_UIElements insertPersonUIScreen;

	insertPersonUIScreen.buttonBack = buttonInit((Rectangle) {20, 20, 100, 30}, "Back");
	insertPersonUIScreen.tbName = textBoxInit(
		(Rectangle){20, insertPersonUIScreen.buttonBack.bounds.height + fontSize + 30, 300, 30},
		"Name:",
		INPUT_TEXT,
		0
	);
	insertPersonUIScreen.tbCpf = textBoxInit(
		(Rectangle){20, insertPersonUIScreen.tbName.bounds.y + insertPersonUIScreen.tbName.bounds.height + fontSize + 10, 300, 30},
		"CPF:",
		INPUT_INTEGER,
		11
	);
	insertPersonUIScreen.tbAge = textBoxInit(
		(Rectangle){20, insertPersonUIScreen.tbCpf.bounds.y + insertPersonUIScreen.tbCpf.bounds.height + fontSize + 10, 300, 30},
		"Age:",
		INPUT_INTEGER,
		3
	);
	insertPersonUIScreen.tbHealthStatus = textBoxInit(
		(Rectangle){20, insertPersonUIScreen.tbAge.bounds.y + insertPersonUIScreen.tbAge.bounds.height + fontSize + 10, 300, 30},
		"Health Status:",
		INPUT_TEXT,
		0
	);
	insertPersonUIScreen.tbNeeds = textBoxInit(
		(Rectangle){20, insertPersonUIScreen.tbHealthStatus.bounds.y + insertPersonUIScreen.tbHealthStatus.bounds.height + fontSize + 10, 300, 30}, 
		"Needs:",
		INPUT_TEXT,
		0
	);
	insertPersonUIScreen.ddbGender = dropDownBoxInit(
		(Rectangle){20, insertPersonUIScreen.tbNeeds.bounds.y + insertPersonUIScreen.tbNeeds.bounds.height + fontSize + 10, 200, 30},
		"Other;Male;Female",
		"Gender"
	);
	insertPersonUIScreen.buttonSubmit = buttonInit((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	insertPersonUIScreen.buttonRetrieve = buttonInit((Rectangle) {insertPersonUIScreen.buttonSubmit.bounds.x + insertPersonUIScreen.buttonSubmit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");
	insertPersonUIScreen.showHealthPopup = false;
	insertPersonUIScreen.showNeedsPopup = false;
    memset(&insertPersonUIScreen.personRetrieved, 0, sizeof(Person));
	// Only set the bounds of the panel, draw everything inside based on it on the draw insert person screen function
	insertPersonUIScreen.panelBounds = (Rectangle) {window_width / 2 - 200, 10, 275, 200};

	// End initializing the insert person screen

	Rectangle styleOptionsBounds = {window_width - 130, 30, 120, 30};
	Rectangle styleOptionsLabelBounds = {styleOptionsBounds.x, styleOptionsBounds.y - 25, styleOptionsBounds.width, 20};

	// Setting the initial state for screen and error code
	ErrorCode error = NIL;
	AppState appState = STATE_MAIN_MENU;

	while (!WindowShouldClose()) {
		// Update
		//----------------------------------------------------------------------------------
		GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

		if (IsWindowResized()) {
			window_width = GetScreenWidth();
			window_height = GetScreenHeight();
			insertPersonUIScreen.buttonSubmit.bounds.y = window_height - 100;
			insertPersonUIScreen.buttonRetrieve.bounds.y = window_height - 100;
			insertPersonUIScreen.panelBounds.x = window_width / 2 - 200;
			styleOptionsBounds.x = window_width - 130;
		}

		if (visualStyleActive != prevVisualStyleActive)
        {
            // Reset to default internal style
            // NOTE: Required to unload any previously loaded font texture
            GuiLoadStyleDefault();

            switch (visualStyleActive)
            {
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

            prevVisualStyleActive = visualStyleActive;
        }
		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		// Visuals options
		GuiLabel(styleOptionsLabelBounds, "Style:");
		GuiComboBox(styleOptionsBounds, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &visualStyleActive);

		switch (appState) {
        case STATE_MAIN_MENU:
            // Draw main menu
            GuiLabel((Rectangle){20, 20, 120, 30}, "Main Menu");
            if (GuiButton((Rectangle){20, 60, 120, 30}, "Insert Person")) {
                appState = STATE_INSERT_PERSON;
            }
            break;
        
        case STATE_INSERT_PERSON:
            DrawInsertPersonScreen(&insertPersonUIScreen, &appState, &error);
            break;
		
		case STATE_INSERT_FOOD:
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

void DrawInsertPersonScreen(INSERT_PERSON_UIElements *ui, AppState *state, ErrorCode *error) {

	// Start draw UI elements

	textBoxDraw(&ui->tbName);
	textBoxDraw(&ui->tbCpf);
	textBoxDraw(&ui->tbAge);

	dropDownBoxDraw(&ui->ddbGender);

	textBoxDraw(&ui->tbHealthStatus);
	textBoxDraw(&ui->tbNeeds);

	// Info Panel
	GuiPanel(ui->panelBounds, TextFormat("CPF info retrieved: %s", ui->personRetrieved.cpf));
	GuiLabel((Rectangle){ui->panelBounds.x + 10, ui->panelBounds.y + 30, 280, 20}, TextFormat("Name: %s", ui->personRetrieved.name));
	GuiLabel((Rectangle){ui->panelBounds.x + 10, ui->panelBounds.y + 60, 280, 20}, TextFormat("Age: %d", ui->personRetrieved.age));

	if (GuiButton((Rectangle){ui->panelBounds.x + 10 + 235, ui->panelBounds.y + 90, 20, 20}, "?")) {ui->showHealthPopup = true;}
	GuiLabel((Rectangle){ui->panelBounds.x + 10, ui->panelBounds.y + 90, 280, 20}, TextFormat("Health Status: %.15s...", ui->personRetrieved.healthStatus));
	
	if (GuiButton((Rectangle){ui->panelBounds.x + 10 + 235, ui->panelBounds.y + 120, 20, 20}, "?")) {ui->showNeedsPopup = true;}
	GuiLabel((Rectangle){ui->panelBounds.x + 10, ui->panelBounds.y + 120, 280, 20}, TextFormat("Needs: %.15s...", ui->personRetrieved.needs));

	GuiLabel((Rectangle){ui->panelBounds.x + 10, ui->panelBounds.y + 150, 280, 20}, TextFormat("Gender: %s", ui->personRetrieved.gender == GENDER_OTHER ? "Other" : ui->personRetrieved.gender == GENDER_MALE ? "Male" : "Female"));
	
	if (ui->showHealthPopup) {
		char wrappedText[MAX_INPUT + 16] = {0}; // +16 to prevent overflow from adding '\n'
		WrapText(ui->personRetrieved.healthStatus, wrappedText, 35);
		int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 250},
								   "#191#Full Health Status", wrappedText, "Close");
		if (result >= 0) ui->showHealthPopup = false;  // Close popup when "Close" is pressed
	}

	if (ui->showNeedsPopup) {
		char wrappedText[MAX_INPUT + 16] = {0}; // +16 to prevent overflow from adding '\n'
		WrapText(ui->personRetrieved.needs, wrappedText, 35);
		int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2, 300, 250},
								   "#191#Full Needs", wrappedText, "Close");
		if (result >= 0) ui->showNeedsPopup = false;  // Close popup when "Close" is pressed
	}

	// End draw UI elements

	// Start button actions

	if (buttonDrawUpdt(&ui->buttonBack)) {
		*state = STATE_MAIN_MENU;
	}

	if (buttonDrawUpdt(&ui->buttonSubmit)) {
		if (*ui->tbCpf.input == '\0') {
			*error = ERROR_CPF_EMPTY;
			fprintf(stderr, "CPF must not be empty.\n");
		} else if (!isValidIntegerInput(ui->tbCpf.input, 11, 11)) {
			*error = ERROR_CPF_NOT_VALID;
			fprintf(stderr, "CPF must be 11 digits.\n");
		} else if (db_check_cpf_exists(ui->tbCpf.input)) {
			*error = ERROR_CPF_ALERADY_EXISTS;
		} else if (db_insert_person(ui->tbCpf.input, ui->tbName.input, atoi(ui->tbAge.input), ui->tbHealthStatus.input, ui->tbNeeds.input, ui->ddbGender.activeOption) != SQLITE_OK) {
			*error = ERROR_DATABASE;
			fprintf(stderr, "Error submitting to database.\n");
		} else {
			*error = NO_ERROR;
		}
	}

	if (buttonDrawUpdt(&ui->buttonRetrieve)) {
		if (db_get_person_by_cpf(ui->tbCpf.input, &ui->personRetrieved)) {
			printf("Retrieved Person - Name: %s, Age: %d, Health Status: %s, Needs: %s, Gender: %d\n", ui->personRetrieved.name, ui->personRetrieved.age, ui->personRetrieved.healthStatus, ui->personRetrieved.needs, ui->personRetrieved.gender);
			*error = NO_ERROR;
		} else {
			*error = ERROR_CPF_NOT_FOUND;
		}
		
	}

	// End button actions

	// Start show warning/error boxes

	// In case updating person
	if (*error == ERROR_CPF_ALERADY_EXISTS) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF Already exists.", "Update;Don't update");
		if (result == 1) {
			if (db_update_person(ui->tbCpf.input, ui->tbName.input, atoi(ui->tbAge.input), ui->tbHealthStatus.input, ui->tbNeeds.input, ui->ddbGender.activeOption) != SQLITE_OK) {
				*error = ERROR_DATABASE;
			}
		}
		if (result >= 0) {
			*error = NO_ERROR;
		}
	}

	if (*error == ERROR_CPF_EMPTY) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must not be empty.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	} else if (*error == ERROR_CPF_NOT_VALID) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must be 11 digits.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	} else if (*error == ERROR_CPF_NOT_FOUND) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF not found.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	} else if (*error == ERROR_DATABASE) {
		int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Error submitting to database.", "OK");
		if (result >= 0) {
			*error = NO_ERROR;
		}
	}

	// End show warning/error boxes

	// Clear the text buffer after handling everything
	if (*error == NO_ERROR) {
		ui->tbName.input[0] = '\0';
		ui->tbCpf.input[0] = '\0';
		ui->tbAge.input[0] = '\0';
		ui->tbHealthStatus.input[0] = '\0';
		ui->tbNeeds.input[0] = '\0';
		*error = NIL;
	}
}
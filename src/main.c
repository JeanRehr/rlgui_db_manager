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
} AppState;

int window_width = 1600;
int window_height = 800;

int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(window_width, window_height, "Raylib + SQLite Example"); // Initialize window with specified dimensions
	GuiSetStyle(DEFAULT, TEXT_SIZE, 15);
	int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);

	// Initialize databases
	if (db_init() != SQLITE_OK) { // Check for successful database initialization
		CloseWindow(); // Close window if database initialization fails
		return -1; // Exit program indicating error
	}

	SetTargetFPS(60); // Set target frames per second for smooth interface

	// TEXTBOXES

	Textbox tbName = textBoxInit(
		(Rectangle){20, 20 + fontSize, 300, 30},
		"Name:",
		INPUT_TEXT,
		0
	);
	Textbox tbCpf = textBoxInit(
		(Rectangle){20, tbName.bounds.y + tbName.bounds.height + fontSize + 10, 300, 30},
		"CPF:",
		INPUT_INTEGER,
		11
	);
	Textbox tbAge = textBoxInit(
		(Rectangle){20, tbCpf.bounds.y + tbCpf.bounds.height + fontSize + 10, 300, 30},
		"Age:",
		INPUT_INTEGER,
		3
	);
	Textbox tbHealthStatus = textBoxInit(
		(Rectangle){20, tbAge.bounds.y + tbAge.bounds.height + fontSize + 10, 300, 30},
		"Health Status:",
		INPUT_TEXT,
		0
	);
	Textbox tbNeeds = textBoxInit(
		(Rectangle){20, tbHealthStatus.bounds.y + tbHealthStatus.bounds.height + fontSize + 10, 300, 30}, 
		"Needs:",
		INPUT_TEXT,
		0
	);

	// END TEXTBOXES

	// DROPDOWNBOXES

	Dropdownbox ddbGender = dropDownBoxInit(
		(Rectangle){20, tbNeeds.bounds.y + tbNeeds.bounds.height + fontSize + 10, 200, 30},
		"Other;Male;Female",
		"Gender"
	);

	// END DROPDOWNBOXES

	// BUTTONS

	Button buttonSubmit = buttonInit((Rectangle) {20, window_height - 100, 100, 30}, "Submit");
	Button buttonRetrieve = buttonInit((Rectangle) {buttonSubmit.bounds.x + buttonSubmit.bounds.width + 10, window_height - 100, 100, 30}, "Retrieve");

	// END BUTTONS

	// PANEL INFO

	Person personRetrieved = {0};
	bool showHealthPopup = false;
	bool showNeedsPopup = false;

	Rectangle panelBounds = {window_width / 2 - 200, 10, 275, 200};

	// END PANEL INFO

	ErrorCode error = NIL;

	while (!WindowShouldClose()) {
		// Update
		//----------------------------------------------------------------------------------
		if (IsWindowResized()) {
			window_width = GetScreenWidth();
			window_height = GetScreenHeight();
			buttonSubmit.bounds.y = window_height - 100;
			buttonRetrieve.bounds.y = window_height - 100;
			panelBounds.x = window_width / 2 - 200;
		}

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		ClearBackground(RAYWHITE);

		// Start draw UI elements
		textBoxDraw(&tbName);
		textBoxDraw(&tbCpf);
		textBoxDraw(&tbAge);

		dropDownBoxDraw(&ddbGender);

		textBoxDraw(&tbHealthStatus);
		textBoxDraw(&tbNeeds);

		// Info Panel
		GuiPanel(panelBounds, TextFormat("CPF info retrieved: %s", personRetrieved.cpf));
		GuiLabel((Rectangle){panelBounds.x + 10, panelBounds.y + 30, 280, 20}, TextFormat("Name: %s", personRetrieved.name));
		GuiLabel((Rectangle){panelBounds.x + 10, panelBounds.y + 60, 280, 20}, TextFormat("Age: %d", personRetrieved.age));

		if (GuiButton((Rectangle){panelBounds.x + 10 + 235, panelBounds.y + 90, 20, 20}, "?")) {showHealthPopup = true;}
		GuiLabel((Rectangle){panelBounds.x + 10, panelBounds.y + 90, 280, 20}, TextFormat("Health Status: %.15s...", personRetrieved.healthStatus));
		
		if (GuiButton((Rectangle){panelBounds.x + 10 + 235, panelBounds.y + 120, 20, 20}, "?")) {showNeedsPopup = true;}
		GuiLabel((Rectangle){panelBounds.x + 10, panelBounds.y + 120, 280, 20}, TextFormat("Needs: %.15s...", personRetrieved.needs));

		GuiLabel((Rectangle){panelBounds.x + 10, panelBounds.y + 150, 280, 20}, TextFormat("Gender: %s", personRetrieved.gender == GENDER_OTHER ? "Other" : personRetrieved.gender == GENDER_MALE ? "Male" : "Female"));
		
		if (showHealthPopup) {
			char wrappedText[MAX_INPUT] = {0};
			WrapText(personRetrieved.healthStatus, wrappedText, 35);
			int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2 - 50, 300, 250},
									   "#191#Full Health Status", wrappedText, "Close");
			if (result >= 0) showHealthPopup = false;  // Close popup when "Close" is pressed
		}

		if (showNeedsPopup) {
			char wrappedText[MAX_INPUT] = {0};
			WrapText(personRetrieved.needs, wrappedText, 35);
			int result = GuiMessageBox((Rectangle){window_width / 2 - 150, window_height / 2, 300, 250},
									   "#191#Full Needs", wrappedText, "Close");
			if (result >= 0) showNeedsPopup = false;  // Close popup when "Close" is pressed
		}

		// End draw UI elements

		// Start button actions
		if (buttonDrawUpdt(&buttonSubmit)) {
			if (*tbCpf.input == '\0') {
				error = ERROR_CPF_EMPTY;
				fprintf(stderr, "CPF must not be empty.\n");
			} else if (!isValidIntegerInput(tbCpf.input, 11, 11)) {
				error = ERROR_CPF_NOT_VALID;
				fprintf(stderr, "CPF must be 11 digits.\n");
			} else if (db_check_cpf_exists(tbCpf.input)) {
				error = ERROR_CPF_ALERADY_EXISTS;
			} else if (db_insert_person(tbCpf.input, tbName.input, atoi(tbAge.input), tbHealthStatus.input, tbNeeds.input, ddbGender.activeOption) != SQLITE_OK) {
				error = ERROR_DATABASE;
				fprintf(stderr, "Error submitting to database.\n");
			} else {
				error = NO_ERROR;
			}
		}

		if (buttonDrawUpdt(&buttonRetrieve)) {
			if (db_get_person_by_cpf(tbCpf.input, &personRetrieved)) {
				printf("Retrieved Person - Name: %s, Age: %d, Health Status: %s, Needs: %s, Gender: %d\n", personRetrieved.name, personRetrieved.age, personRetrieved.healthStatus, personRetrieved.needs, personRetrieved.gender);
				error = NO_ERROR;
			} else {
				error = ERROR_CPF_NOT_FOUND;
			}
			
		}

		// End button actions

		// Start show warning/error boxes

		// In case updating person
		if (error == ERROR_CPF_ALERADY_EXISTS) {
			int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF Already exists.", "Update;Don't update");
			if (result == 1) {
				if (db_update_person(tbCpf.input, tbName.input, atoi(tbAge.input), tbHealthStatus.input, tbNeeds.input, ddbGender.activeOption) != SQLITE_OK) {
					error = ERROR_DATABASE;
				}
			}
			if (result >= 0) {
				error = NO_ERROR;
			}
		}
		

		if (error == ERROR_CPF_EMPTY) {
			int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must not be empty.", "OK");
			if (result >= 0) {
				error = NO_ERROR;
			}
		} else if (error == ERROR_CPF_NOT_VALID) {
			int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF must be 11 digits.", "OK");
			if (result >= 0) {
				error = NO_ERROR;
			}
		} else if (error == ERROR_CPF_NOT_FOUND) {
			int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "CPF not found.", "OK");
			if (result >= 0) {
				error = NO_ERROR;
			}
		} else if (error == ERROR_DATABASE) {
			int result = GuiMessageBox((Rectangle){ window_width / 2 - 150, window_height / 2 - 50, 300, 100 }, "#191#Warning!", "Error submitting to database.", "OK");
			if (result >= 0) {
				error = NO_ERROR;
			}
		}

		// End show warning/error boxes

		// Clear the text buffer after handling everything
		if (error == NO_ERROR) {
			tbName.input[0] = '\0';
			tbCpf.input[0] = '\0';
			tbAge.input[0] = '\0';
			tbHealthStatus.input[0] = '\0';
			tbNeeds.input[0] = '\0';
			error = NIL;
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
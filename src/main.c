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
#include <stdio.h>
#include <string.h>

#include "Button.h"
#include "db_manager.h"
#include "TextBox.h"
#include "CONSTANTS.h"

typedef struct Button Button; // Declare the Button struct type
typedef struct TextBox TextBox; // Declare the TextBox struct type

int TOTAL_PERSONS;  // Total number of persons in the database, used for tracking

// Person data structure for storing individual entries
typedef struct Person {
    char name[MAX_INPUT]; // Name of the person
    long cpf;             // CPF number for unique identification
    int age;              // Age of the person
    char sex;             // Sex of the person (e.g., 'M' or 'F')
    char addr[MAX_INPUT]; // Address of the person
} Person;

// Initializes a Person data structure; currently a stub for further development
void initPerson(Person* person, char* name, long cpf, char* addr, char* birthDate) {
}

typedef struct Food {
    char name[MAX_INPUT];     // Name of the food item
    int quantity;             // Quantity of the food item in stock
    double dailyNeedPerPerson; // Average daily amount of the food needed per person
    int spoilageRate;         // Spoilage rate, e.g., days until spoilage
    char category[MAX_INPUT]; // Category of food (e.g., "Dairy", "Fruits", "Vegetables", "Others")
    time_t entryDate;         // Date the food was entered into the system (for tracking spoilage)
    int expirationDays;       // Number of days from entry date until the food expires
} Food;

int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1600, 800, "Raylib + SQLite Example"); // Initialize window with specified dimensions

    // Initialize database manager
    struct DatabaseManager dbPersons;
    if (dbmanInit(&dbPersons, "persons.db") != SQLITE_OK) { // Check for successful database initialization
        CloseWindow(); // Close window if database initialization fails
        return -1; // Exit program indicating error
    }

    // Initialize TextBox for person name input
    TextBox personNameBox;
    textBoxInit(&personNameBox, 10, FONT_SIZE + 10, 400, 30, "Name", BLACK, LIGHTGRAY, DARKGRAY, BLUE);

    // Initialize TextBox for CPF input
    TextBox personCpfBox;
    textBoxInit(&personCpfBox, 10, personNameBox.rect.y + personNameBox.rect.height + FONT_SIZE + 10, 400, 30, "CPF", BLACK, LIGHTGRAY, DARKGRAY, BLUE);

    // Initialize Button for inserting user entries into the database
    Button buttonInsert;
    buttonInit(&buttonInsert, 10, 600, 200, 40, "Insert", BLACK, LIGHTGRAY, DARKGRAY, DARKGRAY);

    // Initialize Button for retrieving user entries from the database
    Button buttonRetrieve;
    buttonInit(&buttonRetrieve, buttonInsert.rect.width + 25, 600, 200, 40, "Retrieve users", BLACK, LIGHTGRAY, DARKGRAY, DARKGRAY);

    SetTargetFPS(60); // Set target frames per second for smooth interface

    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        // Focus and input handling for person name TextBox
        textBoxCheckFocus(&personNameBox);
        textBoxGetInput(&personNameBox);

        // Focus and input handling for CPF TextBox
        textBoxCheckFocus(&personCpfBox);
        textBoxGetInput(&personCpfBox);

        // Click handling for Insert and Retrieve buttons
        buttonCheckClick(&buttonInsert);
        buttonCheckClick(&buttonRetrieve);

        // Check if Insert button is clicked
        if (buttonCheckClick(&buttonInsert)) {
            // Insert user into the database using input from personNameBox
            if (dbmanInsertUser(&dbPersons, personNameBox.input) == SQLITE_OK) {
                printf("Inserted user: %s\n", personNameBox.input);
                TOTAL_PERSONS += 1; // Increment total persons count
                // Clear input text after insertion
                memset(personNameBox.input, 0, MAX_INPUT); // Clear the entire buffer
                personNameBox.currentCountChar = 0; // Reset count of characters
                personNameBox.input[personNameBox.currentCountChar] = '\0'; // Terminate string properly
            }
        }

        // Check if Retrieve button is clicked to fetch and display users from the database
        if (buttonCheckClick(&buttonRetrieve)) {
            dbmanRetrieveUser(&dbPersons);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw text boxes
        textBoxDraw(&personNameBox);
        textBoxDraw(&personCpfBox);

        // Draw buttons
        buttonDraw(&buttonInsert);
        buttonDraw(&buttonRetrieve);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-initialization
    //--------------------------------------------------------------------------------------
    dbmanClose(&dbPersons);
    CloseWindow();
    //--------------------------------------------------------------------------------------
    return 0; // Exit program
}
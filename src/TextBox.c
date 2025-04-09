#include <raygui.h>
#include <string.h>

#include "Textbox.h"
#include "utilsfn.h"

typedef enum InputType InputType;

typedef struct Textbox Textbox;

Textbox textBoxInit(Rectangle bounds, const char *label, enum InputType type, int maxLength)
{
	Textbox textbox = {0}; // Initialize struct with zeroed memory
	// input already initialized to 0 with above
	textbox.bounds = bounds;
	textbox.editMode = false;
	textbox.label = label;
	textbox.type = type;
	textbox.maxLength = maxLength;
	return textbox;
}

void textBoxDraw(Textbox *textbox)
{
	// Draw label above the textbox
	GuiLabel((Rectangle){textbox->bounds.x, textbox->bounds.y - 20, textbox->bounds.width, 20}, textbox->label);

	// Draw and manage textbox
	if (GuiTextBox(textbox->bounds, textbox->input, MAX_INPUT, textbox->editMode)) {
		textbox->editMode = !textbox->editMode;
	}

	// Filter input based on the specified type
	if (textbox->editMode) {
		switch (textbox->type) {
		case INPUT_INTEGER:
			filterIntegerInput(textbox->input, textbox->maxLength);
			break;
		case INPUT_TEXT:
			// No filtering needed for general text
			break;
		}
	}
}
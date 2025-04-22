#include <external/raylib/raygui.h>
#include <string.h>

#include "CONSTANTS.h"
#include "ui_elements/textbox.h"
#include "utilsfn.h"

typedef enum input_type input_type;

typedef struct textbox textbox;

textbox textbox_init(Rectangle bounds, const char *label, enum input_type type, int max_len_int_input)
{
	textbox textbox = {0}; // Initialize struct with zeroed memory
	// input already initialized to 0 with above
	textbox.bounds = bounds;
	textbox.edit_mode = false;
	textbox.label = label;
	textbox.type = type;
	textbox.max_len_int_input = max_len_int_input;
	return textbox;
}

void textbox_draw(textbox *textbox)
{
	// Draw label above the textbox
	GuiLabel((Rectangle){textbox->bounds.x, textbox->bounds.y - (FONT_SIZE + 5), textbox->bounds.width, 20},
			 textbox->label);

	// Draw and manage textbox
	if (GuiTextBox(textbox->bounds, textbox->input, MAX_INPUT, textbox->edit_mode)) {
		textbox->edit_mode = !textbox->edit_mode;
	}

	// Filter input based on the specified type
	if (textbox->edit_mode) {
		switch (textbox->type) {
		case INPUT_INTEGER:
			filter_integer_input(textbox->input, textbox->max_len_int_input);
			break;
		case INPUT_TEXT:
			// No filtering needed for general text
			break;
		}
	}
}

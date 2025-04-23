#include <external/raylib/raygui.h>

#include <string.h>

#include "ui_elements/textboxsecret.h"

typedef struct textboxsecret textboxsecret;

textboxsecret textboxsecret_init(Rectangle bounds, const char *label)
{
	textboxsecret tbs = {0}; // Initialize struct with zeroed memory
	// inputs already initialized to 0 with above
	tbs.bounds = bounds;
	tbs.secret_view = false;
	tbs.edit_mode = false;
	tbs.label = label;
	return tbs;
}

void textboxsecret_draw(textboxsecret *tbs)
{
	// Draw label above the textboxsecret
	GuiLabel((Rectangle){tbs->bounds.x, tbs->bounds.y - (FONT_SIZE + 5), tbs->bounds.width, 20}, tbs->label);

	// Handle the text box input
	if (GuiTextBox(tbs->bounds, tbs->input, MAX_INPUT, tbs->edit_mode)) {
		tbs->edit_mode = !tbs->edit_mode;
	}

	// Update display text based on secret_view mode
	if (tbs->secret_view) {
		strcpy(tbs->display_input, tbs->input);
	} else {
		memset(tbs->display_input, '*', strlen(tbs->input));
		tbs->display_input[strlen(tbs->input)] = '\0';
	}

	// Draw the display text (overlay)
	GuiLabel(tbs->bounds, tbs->display_input);
	// Draw show/hide toggle button
	Rectangle btnBounds = {tbs->bounds.x + tbs->bounds.width - 25, tbs->bounds.y + (tbs->bounds.height - 20) / 2 - 10,
						   20, 20};

	if (GuiLabelButton(btnBounds, tbs->secret_view ? "#137#" : "#138#")) {
		tbs->secret_view = !tbs->secret_view;
	}
}
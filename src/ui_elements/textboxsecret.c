#include <external/raylib/raygui.h>

#include <string.h>

#include "ui_elements/textboxsecret.h"

struct textboxsecret textboxsecret_init(Rectangle bounds, const char *label)
{
	struct textboxsecret tbs = {0}; // Initialize struct with zeroed memory
	// inputs already initialized to 0 with above
	tbs.bounds = bounds;
	tbs.secret_view = false;
	tbs.label = label;
	return tbs;
}

void textboxsecret_draw(struct textboxsecret *tbs)
{
	// Draw label above the textboxsecret
	GuiLabel((Rectangle){tbs->bounds.x, tbs->bounds.y - (FONT_SIZE + 5), tbs->bounds.width, 20}, tbs->label);

	GuiTextBoxSecret(tbs->bounds, tbs->input, MAX_INPUT, &tbs->secret_view);
}
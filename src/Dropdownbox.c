#include <raygui.h>
#include "Dropdownbox.h"

typedef struct Dropdownbox Dropdownbox;

Dropdownbox dropDownBoxInit(Rectangle bounds, const char *options, const char *label)
{
	Dropdownbox ddb = {0};
	ddb.bounds = bounds;
	ddb.options = options;
	ddb.label = label;
	ddb.activeOption = 0;
	ddb.editMode = false;
	return ddb;
}

void dropDownBoxDraw(Dropdownbox *ddb)
{
	GuiLabel((Rectangle){ddb->bounds.x, ddb->bounds.y - 20, ddb->bounds.width, 20}, ddb->label);

	if (GuiDropdownBox(ddb->bounds, ddb->options, &ddb->activeOption, ddb->editMode)) {
		ddb->editMode = !ddb->editMode;
	}
}
#include <raygui.h>

#include <stddef.h>

#include "CONSTANTS.h"
#include "intbox.h"

typedef struct intbox intbox;

intbox intbox_init(Rectangle bounds, const char *label, const int min_val, const int max_val)
{
	intbox ib = {0};
	ib.bounds = bounds;
	ib.label = label;
	ib.input = 0;
	ib.edit_mode = false;
	ib.min_val = min_val;
	ib.max_val = max_val;
	return ib;
}

void intbox_draw(intbox *ib)
{
	// Check whether the provided width of the box is higher than the width of the title.
	// If not, use the width of the title as width of the GuiLabel
	int label_width = MeasureText(ib->label, FONT_SIZE) > ib->bounds.width ? MeasureText(ib->label, FONT_SIZE) : ib->bounds.width;
	// Draw label above the intbox
	GuiLabel((Rectangle){ib->bounds.x, ib->bounds.y - (FONT_SIZE + 5), label_width, 20}, ib->label);

	if (GuiValueBox(ib->bounds, NULL, &ib->input, ib->min_val, ib->max_val, ib->edit_mode)) {
		ib->edit_mode = !ib->edit_mode;
	}
}

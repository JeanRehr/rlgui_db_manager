#include <raygui.h>

#include <stdbool.h>
#include <string.h>

#include "button.h"
#include "CONSTANTS.h"

typedef struct button button;

// Function to initialize a Button with given properties
button button_init(Rectangle bounds, const char *label)
{
	button button = {0};

	// Calculate the width of the label
	int label_width = MeasureText(label, FONT_SIZE);
	if (label_width > bounds.width) {
		button.bounds.x = bounds.x;
		button.bounds.y = bounds.y;
		button.bounds.width = label_width + 10; // +10 to give it some space
		button.bounds.height = bounds.height;
	} else {
		button.bounds = bounds;
	}

	button.label = label;
	return button;
}

int button_draw_updt(button *button)
{
	return GuiButton(button->bounds, button->label);
}

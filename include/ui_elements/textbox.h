#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <external/raylib/raylib.h>

#include "CONSTANTS.h"

struct textbox {
	Rectangle bounds;
	char input[MAX_INPUT];
	bool edit_mode;
	const char *label;
};

struct textbox textbox_init(Rectangle bounds, const char *label);

void textbox_draw(struct textbox *tb);

#endif // TEXTBOX_H

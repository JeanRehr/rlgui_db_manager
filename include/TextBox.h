#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <raylib.h>

#include "CONSTANTS.h"

enum input_type {
	INPUT_INTEGER = 0,
	INPUT_TEXT,
};

struct textbox {
	Rectangle bounds;
	char input[MAX_INPUT];
	bool edit_mode;
	const char *label;
	enum input_type type;
	int max_len_int_input;
};

struct textbox textbox_init(Rectangle bounds, const char *label, enum input_type type, int max_len_int_input);

void textbox_draw(struct textbox *tb);

#endif // TEXTBOX_H
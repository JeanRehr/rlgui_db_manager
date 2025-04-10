#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <raylib.h>

#include "CONSTANTS.h"

enum InputType {
	INPUT_INTEGER = 0,
	INPUT_TEXT,
};

struct Textbox {
	Rectangle bounds;
	char input[MAX_INPUT];
	bool editMode;
	const char *label;
	enum InputType type;
	int maxLengthIntInput;
};

struct Textbox textBoxInit(Rectangle bounds, const char *label, enum InputType type, int maxLengthIntInput);

void textBoxDraw(struct Textbox *textbox);

#endif //TEXTBOX_H
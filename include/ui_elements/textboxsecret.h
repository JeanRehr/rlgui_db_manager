#ifndef TEXTBOXSECRET_H
#define TEXTBOXSECRET_H

#include <external/raylib/raylib.h>

#include "CONSTANTS.h"

struct textboxsecret {
	Rectangle bounds;
	char input[MAX_INPUT]; // Actual storage of the input
	char display_input[MAX_INPUT]; // Text to display (asterisks)
	bool secret_view; // Toggle to display the actual text
	bool edit_mode;
	const char *label;
};

struct textboxsecret textboxsecret_init(Rectangle bounds, const char *label);

void textboxsecret_draw(struct textboxsecret *tbs);

#endif // TEXTBOXSECRET_H
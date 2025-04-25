#ifndef TEXTBOXINT_H
#define TEXTBOXINT_H

#include <external/raylib/raylib.h>

#include "CONSTANTS.h"

// For when there is the need to keep leading zeroes, but input must only integer
struct textboxint {
	Rectangle bounds;
	char input[MAX_INPUT];
	const char *label;
	bool edit_mode;
};

struct textboxint textboxint_init(Rectangle bounds, const char *label);

// In the textboxint_draw there will be a filter to only allow input to be numbers
void textboxint_draw(struct textboxint *tbi);

#endif // TEXTBOXINT_H
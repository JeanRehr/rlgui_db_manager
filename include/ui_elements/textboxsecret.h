#ifndef TEXTBOXSECRET_H
#define TEXTBOXSECRET_H

#include <external/raylib/raylib.h>

#include "CONSTANTS.h"

struct textboxsecret {
	Rectangle bounds;
	char input[MAX_INPUT];
	bool edit_mode;
	const char *label;
};

struct textboxsecret textboxsecret_init(Rectangle bounds, const char *label);

void textboxsecret_draw(struct textboxsecret *tbs);

#endif // TEXTBOXSECRET_H
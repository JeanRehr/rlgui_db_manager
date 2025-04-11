#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include <raylib.h>
#include <stdbool.h>

struct dropdownbox {
	Rectangle bounds;
	const char *options;
	const char *label;
	int active_option;
	bool edit_mode;
};

struct dropdownbox dropdownbox_init(Rectangle bounds, const char *options, const char *label);

void dropdownbox_draw(struct dropdownbox *ddb);

#endif // DROPDOWNBOX_H
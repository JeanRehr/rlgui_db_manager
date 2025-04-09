#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include <raylib.h>
#include <stdbool.h>

struct Dropdownbox {
	Rectangle bounds;
	const char *options;
	const char *label;
	int activeOption;
	bool editMode;
};

struct Dropdownbox dropDownBoxInit(Rectangle bounds, const char *options, const char *label);

void dropDownBoxDraw(struct Dropdownbox *ddb);

#endif //DROPDOWNBOX_H
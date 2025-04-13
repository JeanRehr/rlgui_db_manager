#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <raylib.h>

#include <stdbool.h>

struct checkbox {
	Rectangle bounds;
	const char *title;
    bool checked;
};

struct checkbox checkbox_init(Rectangle bounds, const char *title);

int checkbox_draw(struct checkbox *checkbox);

#endif // CHECKBOX_H
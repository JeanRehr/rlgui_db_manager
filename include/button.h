#ifndef BUTTON_H
#define BUTTON_H

#include <raylib.h>

#include <stdbool.h>

struct button {
	Rectangle bounds;
	const char *label;
};

// If no width (bounds.width) is specified when initiating the button, or a width which is less than the text width of
// label, then this will calculate and pass to the button width the width of the text
struct button button_init(Rectangle bounds, const char *label);

int button_draw_updt(struct button *button);

#endif // BUTTON_H

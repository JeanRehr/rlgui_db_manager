#ifndef FLOATBOX_H
#define FLOATBOX_H

#include <raylib.h>

#include <stdbool.h>

#include "CONSTANTS.h"

struct floatbox {
	Rectangle bounds;
	const char *title;
    char text_input[MAX_INPUT];
    float value;
    bool edit_mode;
};

struct floatbox floatbox_init(Rectangle bounds, const char *title);

void floatbox_draw(struct floatbox *fb);

#endif // FLOATBOX_H
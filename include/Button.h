#ifndef BUTTON_H
#define BUTTON_H

#include <raylib.h>

#include <stdbool.h>

struct Button {
    Rectangle bounds;
    const char* label;
};

struct Button buttonInit(Rectangle bounds, const char* label);

int buttonDrawUpdt(struct Button *button);

#endif //BUTTON_H
#ifndef INTBOX_H
#define INTBOX_H

#include <external/raylib/raylib.h>

#include <stdbool.h>

struct intbox {
    Rectangle bounds;
    const char *label;
    int input;
    int min_val;
    int max_val;
    bool edit_mode;
};

struct intbox intbox_init(Rectangle bounds, const char *label, const int min_val, const int max_val);

void intbox_draw(struct intbox *ib);

#endif // INTBOX_H
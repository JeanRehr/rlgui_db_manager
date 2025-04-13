#include <raygui.h>

#include <stdbool.h>

#include "checkbox.h"

typedef struct checkbox checkbox;

checkbox checkbox_init(Rectangle bounds, const char *title)
{
    checkbox cb = {0};
    cb.bounds = bounds;
    cb.title = title;
    cb.checked = false;
    return cb;
}

int checkbox_draw(struct checkbox *cb)
{
	return GuiCheckBox(cb->bounds, cb->title, &cb->checked);
}